#include "appdefs.h"
#include "fs_stream.h"

#include "cs.h"
#include "init.h"
#include "display.h"
#include "libopencm3/stm32/gpio.h"

TFsStreamTask *FsStreamTask;
volatile uint8_t FsStream_enable_fl = 0;

TFsStreamTask::TFsStreamTask(const char *name, const int stack_size, const int priority)
		: TTask(name, stack_size, priority, true)
{

}

void TFsStreamTask::Code()
{
//	mount();
	FRESULT fr = f_mount(&fs, "SD", 1);

	browser.play_list_folder = emb_string("/PLAYLIST/Default");

	while(!FsStream_enable_fl);

	Delay(500);
	DisplayTask->Clear();
	DisplayTask->StringOut(0, 0, (uint8_t*) "Checking Folders");
	Delay(500);

	DIR dir;
	if((fr = f_opendir(&dir, "/PLAYLIST")) == FR_OK)
	{
		f_closedir(&dir);
	}
	else
	{
		while(1)
		{
			if (f_mkdir("/PLAYLIST") == FR_OK)
				break;
		}
	}

	if (f_opendir(&dir, "/SONGS") == FR_OK)
	{
		f_closedir(&dir);
	}
	else
	{
		while (1)
		{
			if (f_mkdir("/SONGS") == FR_OK)
				break;
		}
	}
	FsStream_enable_fl = 0;

	FIL fsys;
	UINT br;
	if (f_open(&fsys, "/system.ego", FA_READ) == FR_OK)
	{
		f_read(&fsys, sys_param, 64, &br);
		f_lseek(&fsys, 64);
		f_gets(browser.buf, FF_MAX_SS, &fsys);

		selectedSong.trackPath[0] = browser.buf;

		if (!selectedSong.trackPath[0].empty())
		{
			browser.play_list_folder = selectedSong.trackPath[0].c_str();
		}
		else
		{
			selectAnyFindedPlaylist();
		}
	}
	else
	{
		f_open(&fsys, "/system.ego", FA_OPEN_ALWAYS | FA_READ);
		memset(sys_param, 0, 64);
		f_write(&fsys, sys_param, 64, &br);
		selectAnyFindedPlaylist();
	}

	f_close(&fsys);
	if (f_open(&fsys, "/ctrl.ego", FA_READ) == FR_OK)
	{
		f_read(&fsys, ctrl_param, 32, &br);
	}
	else
	{
		f_open(&fsys, "/ctrl.ego", FA_OPEN_ALWAYS | FA_READ);
		memset(ctrl_param, 0, 32);
		f_write(&fsys, ctrl_param, 32, &br);
	}
	f_close(&fsys);

	if (f_open(&fsys, "/midi_map.ego", FA_READ) == FR_OK)
	{
		f_read(&fsys, pc_param, 256, &br);
	}
	else
	{
		f_open(&fsys, "/midi_map.ego", FA_OPEN_ALWAYS | FA_READ);
		memset(pc_param, 0, 256);
		for (uint8_t i = 0; i < 128; i++)
			pc_param[i * 2 + 1] = i;
		f_write(&fsys, pc_param, 256, &br);
	}
	f_close(&fsys);

	while (1)
	{
		// ожидания уведомления о запросе данных
		query_notify_t qn;
		NotifyWait(0, 0, (uint32_t*) &qn, portMAX_DELAY);
		switch (qn.notify)
		{
		case qn_wav_data:
			getWavData();
			break;
		case qn_midi_events:
			getMidiEvents();
			break;
		case qn_prev:
			prev();
			break;
		case qn_next:
			next();
			break;
		case qn_action:
			action(qn.action_param, qn.play_index);
			break;
		default:
		{
		}
		}

		TScheduler::Yeld();
	}
}

void TFsStreamTask::selectAnyFindedPlaylist()
{
	FRESULT res;
	DIR dir;
	FILINFO fno;

	res = f_opendir(&dir, "/PLAYLIST");
	if(res != FR_OK) return;
	while (1)
	{
		res = f_readdir(&dir, &fno);
		if (res != FR_OK || fno.fname[0] == 0) break;

		// Фильтруем только папки (исключая . и ..)
		if ((fno.fattrib & AM_DIR) &&
			fno.fname[0] != '.' &&
			!(fno.fname[0] == '.' && fno.fname[1] == '.'))
		{
			// Выбираем первую найенную папку
			emb_printf::sprintf(browser.play_list_folder , "/PLAYLIST/%s", fno.fname);
			break;
		}
	}
	f_closedir(&dir);

	if(browser.play_list_folder.empty())
	{
		while (1)
		{
			if(f_mkdir("/PLAYLIST/Default") == FR_OK)
				break;
		}
		browser.play_list_folder = "/PLAYLIST/Default";
	}


}

//---------------------------------------events----------------------
void TFsStreamTask::getWavData()
{
	// чтение данных из файла wave и запись в буффер
	size_t br = 0;

	f_read(&selectedSong.wavFile[0], target->dest_sound, target->size, &br);
	size_t zeros = target->size - br;

	if(zeros)
	{
		memset(target->dest_sound + br, 0, zeros);
	}

	// чтение данных из файла click и запись в буффер
	br = 0;

	f_read(&selectedSong.wavFile[1], target->dest_click, target->size, &br);
	zeros = target->size - br;
	if(zeros)
	{
		memset(target->dest_click + br, 0, zeros);
	}

	selectedSong.read_chunk_count++;
}

void TFsStreamTask::getMidiEvents()
{
	midiPlayer.readEvents(midiStartInterval, midiStopInterval);
}
//-------------------------------------------------------------------
void TFsStreamTask::enter_dir(const char *name, const char *high_level_node, bool begin = false)
{
	FRESULT fr;

	if (!begin)
	{
		if ((fr = f_closedir(&browser.dir)) != FR_OK)
		{
			////rmsg( ConsoleTask->ReadLine(),"error :%s\n" , f_err2str(fr));
			enter_dir("/", "", true);
		}
	}

	if ((fr = f_opendir(&browser.dir, name)) != FR_OK)
	{
		//rmsg( ConsoleTask->ReadLine(),"error :%s\ngo to IMPULSE" , f_err2str(fr));
		enter_dir("/", "", true);
		return;
	}

	if ((fr = f_chdir(name)) != FR_OK)
	{
		//rmsg( ConsoleTask->ReadLine(),"error :%s\n" , f_err2str(fr));
		enter_dir("/", "", true);
		return;
	}

	fr = f_getcwd(browser.buf, FF_MAX_LFN);
	if (name[0] == '.' && name[1] == '.' && name[2] == 0)
	{
		browser.tmp = high_level_node;
		while (((fr = f_readdir(&browser.dir, &browser.fno)) == FR_OK)
				&& (browser.tmp != browser.fno.fname) && (browser.fno.fname[0]))
		{
		}
		if (fr != FR_OK)
		{
			//rmsg( ConsoleTask->ReadLine(),"error :%s\n" , f_err2str(fr));
			enter_dir("/", "", true);
		}
	}
	else
	{
		if ((fr = f_readdir(&browser.dir, &browser.fno)) != FR_OK)
		{
			//rmsg( ConsoleTask->ReadLine(),"error :%s\n" , f_err2str(fr));
			enter_dir("/", "", true);
		}
	}
}

bool TFsStreamTask::currentPathIsDirectory()
{
	return browser.fno.fattrib & AM_DIR;
}

void TFsStreamTask::action(action_param_t val, uint8_t play_index)
{
	FRESULT fr;

	if(val == enter_directory)
	{
		if(browser.fno.fattrib & AM_DIR)
		{
			// entry to dir
			browser.tmp = emb_string(browser.fno.fname);
			if (browser.tmp == "..")
			{
				fr = f_getcwd(browser.buf, FF_MAX_LFN);
				browser.tmp = browser.buf;
				size_t slash_pos = browser.tmp.find_last_of('/');
				browser.tmp = browser.tmp.substr(slash_pos + 1,
						browser.tmp.length());
			}

			// entry to dir
			enter_dir(browser.fno.fname, browser.tmp.c_str());
		}
		return;
	}

	if(val == save_song)
	{
		// file action
		DIR d;
		if ((fr = f_opendir(&d, browser.play_list_folder.c_str())) != FR_OK)
		{
			fr = f_mkdir(browser.play_list_folder.c_str());
		}
		else
		{
			fr = f_closedir(&d);
		}
		FIL f;

		emb_string songPath;
		emb_printf::sprintf(songPath, "%s/%1.ego", browser.play_list_folder.c_str(), play_index);
		editingSong.save(songPath);
	}
}

void TFsStreamTask::next()
{
	FRESULT fr;

	while(1)
	{
		FILINFO fno;
		FRESULT res = f_readdir(&browser.dir, &fno);

		if(res != FR_OK || fno.fname[0] == 0)
		{
			break;
		}

		if(fno.fname[0] == '.')
		{
			continue;
		}
		else
		{
			browser.fno = fno;
			break;
		}
	}
}

void TFsStreamTask::prev()
{
	browser.tmp = browser.fno.fname;
	size_t index = 0;
	// проход c вычисленем индекса текущего файла
	f_readdir(&browser.dir, NULL);

	FILINFO fno;

	while(f_readdir(&browser.dir, &browser.fno) == FR_OK && browser.fno.fname[0])
	{
		index++;
		if(browser.tmp == browser.fno.fname)
		{
			index--;
			if(index)
			{
				// второй проход по индексу -1 с получением имени передидущего файла
				f_readdir(&browser.dir, NULL);
				for (size_t i = 0; i < index; i++)
				{
					f_readdir(&browser.dir, &browser.fno);
//					if(browser.fno.fname[0] != '.')
//						continue;
				}
			}
			return;
		}
	}
}

void TFsStreamTask::getWavName(emb_string &dir_path, emb_string &file_path, emb_string &mask)
{
	DIR dir; /* Directory search object */
	FILINFO fno; /* File information */

	FRESULT fr = f_findfirst(&dir, &fno, dir_path.c_str(), mask.c_str()); /* Start to search for photo files */
	if (fr == FR_OK && fno.fname[0]) /* Repeat while an item is found */
	{
		file_path = dir_path + "/" + fno.fname;
		f_closedir(&dir);
	}
	else
	{
		file_path.clear();
	}
}
