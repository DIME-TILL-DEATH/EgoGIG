#include "appdefs.h"
#include "fs_stream.h"

#include "cs.h"
#include "init.h"
#include "display.h"
#include "libopencm3/stm32/gpio.h"

TFsStreamTask *FsStreamTask;
volatile uint8_t FsStream_enable_fl = 0;

void TFsStreamTask::Code()
{
	mount();

	browser.play_list_folder = emb_string("/PLAYLIST/Default");

	while (!FsStream_enable_fl)
		;
	Delay(500);
	DisplayTask->Clear();
	DisplayTask->StringOut(0, 0, (uint8_t*) "Checking Folders");
	Delay(500);

	DIR dir;
	if ((fr = f_opendir(&dir, "/PLAYLIST")) == FR_OK)
	{
		f_closedir(&dir);
	}
	else
	{
		while (1)
		{
			if (f_mkdir("/PLAYLIST") == FR_OK)
				break;
		}
	}
	if ((fr = f_opendir(&dir, "/PLAYLIST/Default")) == FR_OK)
	{
		f_closedir(&dir);
	}
	else
	{
		while (1)
		{
			if (f_mkdir("/PLAYLIST/Default") == FR_OK)
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
			browser.play_list_folder = selectedSong.trackPath[0].c_str();
	}
	else
	{
		f_open(&fsys, "/system.ego", FA_OPEN_ALWAYS | FA_READ);
		memset(sys_param, 0, 64);
		f_write(&fsys, sys_param, 64, &br);
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

#if __STRETCH_DATA_FILE__
  if(f_open(&fsys,"/stretch.ego",FA_READ) == FR_OK)
    {
	  f_read( &fsys , stretch , sizeof(stretch) , &br);
    }
  f_close(&fsys);
#endif

	while (1)
	{
		// ожидания уведомления о запросе данных
		query_notify_t qn;
		NotifyWait(0, 0, (uint32_t*) &qn, portMAX_DELAY);
		switch (qn.notify)
		{
		case qn_data:
			get();
			break;
		case qn_prev:
			prev();
			break;
		case qn_next:
			next();
			break;
		case qn_curr:
			curr();
			break;
		case qn_action:
			action(qn.action_param, qn.play_index, qn.play_next);
			break;
		default:
		{
		}
		}

		TScheduler::Yeld();
	}
}

//bool TFsStreamTask::is_valid_wave(FIL *file, uint8_t num)
//{
//	wave_header_t wh;
//	UINT br = 0;
//	if ((fr = f_read(file, &wh, 12, &br)) != FR_OK)
//		return false;
//
//	// RIFF
//	if (!(wh.chunkId[0] == 'R' && wh.chunkId[1] == 'I' && wh.chunkId[2] == 'F'
//			&& wh.chunkId[3] == 'F'))
//	{
//		return false;
//	}
//
//	// WAVE
//	if (!(wh.format[0] == 'W' && wh.format[1] == 'A' && wh.format[2] == 'V'
//			&& wh.format[3] == 'E'))
//	{
//		return false;
//	}
//
//	uint32_t find_data = file->fptr;
//	char data_id[4];
//	while (1)
//	{
//		f_lseek(file, find_data++);
//		f_read(file, data_id, 4, &br);
//		if ((data_id[0] == 'f' && data_id[1] == 'm' && data_id[2] == 't'
//				&& data_id[3] == ' '))
//		{
//			f_read(file, &wh.subchunk1Size, 20, &br);
//			break;
//		}
//	}
//
//	if (wh.subchunk1Size != 16 && wh.subchunk1Size != 18)
//		return false;
//	if (wh.audioFormat != 1)
//		return false;
//	if (wh.numChannels != 2)
//		return false;
//	if (wh.sampleRate != 44100)
//		return false;
//	if (wh.bitsPerSample != 16)
//		return false;
//
//	size_t sz;
//	volatile size_t po_data = 36;
//	while (1)
//	{
//
//		f_read(file, data_id, 4, &br);
//
//		if ((data_id[0] == 'd' && data_id[1] == 'a' && data_id[2] == 't'
//				&& data_id[3] == 'a'))
//		{
//			f_read(file, &sz, 4, &br);
//			break;
//		}
//
//		po_data = f_tell(file) - 3;
//		f_lseek(file, po_data);
//	}
//
//	if (!num)
//	{
//		if ((po_data % 6))
//		{
//			f_lseek(file, f_tell(file) + 2);
////			swap_need |= (swap_need_t) snSound;
//		}
//		size = sz;
//	}
//
//	else
//	{
//		//if ( wh.subchunk1Size == 18 ) { f_lseek(file, f_tell(file)+2) ; swap_need |= (swap_need_t)snClick ;}
//		size1 = sz;
//	}
//
//	return true;
//}

//-------------------------------------------------------------------
void TFsStreamTask::enter_dir(const char *name, const char *high_level_node,
		bool begin = false)
{
	if (!begin)
		if ((fr = f_closedir(&browser.dir)) != FR_OK)
		{
			////rmsg( ConsoleTask->ReadLine(),"error :%s\n" , f_err2str(fr));
			enter_dir("/", "", true);
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


void TFsStreamTask::writeSongContent(emb_string& songPath, emb_string& path1, emb_string& path2)
{
	FIL file;
	char temp = 62;

	fr = f_open(&file, songPath.c_str(), FA_OPEN_ALWAYS | FA_WRITE);
	f_lseek(&file, 0);
	f_truncate(&file);

	if (play_next)
		f_write(&file, &temp, 1, fw);
	else
		f_lseek(&file, 0);

	f_puts(path1.c_str(), &file);
	f_puts(path2.c_str(), &file);

	extern volatile uint8_t led_blink_fl;
	led_blink_fl = 1;

	f_close(&file);
}

emb_string str1;
void TFsStreamTask::action(action_param_t val, uint8_t play_index, uint8_t play_next)
{
	if (browser.fno.fattrib & AM_DIR)
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
	else
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
		emb_string str2;


		emb_printf::sprintf(songPath, "%s/%1.ego", browser.play_list_folder.c_str(), play_index);

		if (val == ap_1_wav)
		{
			str1.clear();
			str2.clear();

			f_getcwd(browser.buf, FF_MAX_SS);
			emb_printf::sprintf(str1, "%s/%s\n", browser.buf, browser.fno.fname);

			fr = f_open(&f, str1.c_str(), FA_READ);
//			if (is_valid_wave(&f, 0))
//			{
//				writeSongContent(songPath, str1, str2);
//			}

			f_close(&f);
		}
		else
		{
			str2.clear();

			f_getcwd(browser.buf, FF_MAX_SS);
			emb_printf::sprintf(str2, "%s/%s\n", browser.buf, browser.fno.fname);
			fr = f_open(&f, str2.c_str(), FA_READ);
//			if (is_valid_wave(&f, 1))
//			{
//				writeSongContent(songPath, str1, str2);
//			}

			f_close(&f);
		}
	}
}

void TFsStreamTask::curr()
{
	//rmsg( ConsoleTask->ReadLine(),"%s\n" , browser.fno.fname );
}

void TFsStreamTask::next()
{
	DIR dir = browser.dir;
	FILINFO fno = browser.fno;
	if ((fr = f_readdir(&browser.dir, &browser.fno)) == FR_OK)
	{
		if (!browser.fno.fname[0])
		{
			browser.dir = dir;
			browser.fno = fno;
		}
	}
}
void TFsStreamTask::prev()
{
	browser.tmp = browser.fno.fname;
	size_t index = 0;
	// проход c вычисленем индекса текущего файла
	f_readdir(&browser.dir, NULL);

	while ((fr = f_readdir(&browser.dir, &browser.fno)) == FR_OK
			&& browser.fno.fname[0])
	{
		index++;
		if (browser.tmp == browser.fno.fname)
		{
			index--;
			if (index)
			{
				// второй проход по индексу -1 с получением имени передидущего файла
				f_readdir(&browser.dir, NULL);
				for (size_t i = 0; i < index; i++)
					f_readdir(&browser.dir, &browser.fno);
			}
			return;
		}
	}
}
