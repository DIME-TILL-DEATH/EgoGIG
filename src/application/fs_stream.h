#ifndef __FS_STREAM_H__
#define __FS_STREAM_H__

#include "appdefs.h"
#include "ff.h"
#include "sd_storage.h"
#include <vector>

#include "lcd-hd44780.h"

#include "song.h"
#include "init.h"

class TFsStreamTask: public TTask
{
public:
	Song editingSong;
	Song selectedSong;

	struct browser_t
	{
		size_t curr_dir_child_index; // порядковый номер объекта текущей директории в списке объектов родительской директори
		size_t object_index; // порядковый номер объекта файла в списке текущей директории

		DIR dir;
		FILINFO fno;
		char buf[FF_MAX_LFN + 4];
		emb_string tmp;
		emb_string play_list_folder;

	};

	TFsStreamTask(const char *name, const int stack_size, const int priority);

	enum notify_t
	{
		qn_data = 0, qn_next, qn_prev, qn_curr, qn_owner, qn_action
	};
	enum action_param_t
	{
		enter_directory, save_song
	};

	struct query_notify_t
	{
		notify_t notify :8;
		union
		{
			struct
			{
				action_param_t action_param :8;
				uint32_t play_index :8;
			} __attribute__((packed));
		} __attribute__((packed));
	} __attribute__((packed));

	inline void data_notify(const target_t *val)
	{
		target = val;
		query_notify_t qn =
		{ .notify = qn_data };
		notify(qn);
	}
	inline void next_notify()
	{
		query_notify_t qn =
		{ .notify = qn_next };
		notify(qn);
	}
	inline void prev_notify()
	{
		query_notify_t qn =
		{ .notify = qn_prev };
		notify(qn);
	}
	inline void action_notify(action_param_t val, uint8_t play_index)
	{
		query_notify_t qn;
		qn.notify = qn_action;
		qn.action_param = val;
		qn.play_index = play_index;
		notify(qn);
	}
	inline void curr_notify()
	{
		query_notify_t qn =
		{ .notify = qn_curr };
		notify(qn);
	}
	inline void owner_notify()
	{
		query_notify_t qn =
		{ .notify = qn_owner };
		notify(qn);
	}

	bool currentPathIsDirectory();

	inline error_t delete_track(size_t index)
	{
		emb_string temp;
		emb_printf::sprintf(temp, "%s/%1.ego", browser.play_list_folder.c_str(), index);
		f_unlink(temp.c_str());
		return Song::eOk;
	}

	inline void curr_path(emb_string &dst)
	{
		f_getcwd(browser.buf, FF_MAX_LFN);
		dst = emb_string(browser.buf);
	}

	inline size_t pos()
	{
		return (f_tell(&selectedSong.wavFile[0]) - selectedSong.soundDataOffset[0]) / sizeof(wav_sample_t);
	}

	inline void pos(size_t val)
	{
		for(uint8_t i=0; i<Player::maxTrackCount; i++)
			fr = f_lseek(&selectedSong.wavFile[i], val * sizeof(wav_sample_t) + selectedSong.soundDataOffset[i]);

		midiPlayer.pos(val);
	}

	inline void browser_name(emb_string &dst)
	{
		dst = browser.fno.fname;
	}

	inline bool play_list_folder()
	{
		if ((browser.fno.fattrib & AM_DIR) && (browser.fno.fname[0] != '.')
				&& (browser.fno.fname[1] != '.') && (browser.fno.fname[1] != 0)) //(!strcmp(browser.fno.fname,".."))
		{
			fr = f_getcwd(browser.buf, FF_MAX_SS);
			browser.play_list_folder = emb_string(browser.buf);
			browser.play_list_folder += "/";
			browser.play_list_folder += emb_string(browser.fno.fname);
			FIL fsys;
			f_open(&fsys, "/system.ego", FA_WRITE);
			f_lseek(&fsys, 64);
			f_truncate(&fsys);
			f_puts(browser.play_list_folder.c_str(), &fsys);
			f_close(&fsys);
			return true;
		}
		return false;
	}

	inline void play_list_folder(emb_string &val) const
	{
		val = browser.play_list_folder.substr(
				browser.play_list_folder.find_last_of('/') + 1,
				browser.play_list_folder.length());
	}

	emb_string browserPlaylistFolder() const
	{
		return browser.play_list_folder;
	}


	void enter_dir(const char *name, const char *high_level_node, bool begin);

protected:

	inline bool mount()
	{
		// монтирование раздела
		fr = f_mount(&fs, "SD", 1);
		return fr == FR_OK;
	}

	inline bool umount()
	{
		selectedSong.close();
		editingSong.close();

		// размонтирование раздела
		fr = f_mount(0, "SD", 0);
		sd_storage_deinitialize();
		return fr == FR_OK;
	}

	inline bool is_valid_ff_object(FIL *file)
	{
		if (file->obj.fs)
			return true;
		fr = FR_INVALID_OBJECT;
		return false;
	}


	inline void get()
	{
		// чтение данных из файла wave и запись в буффер
		size_t br = 0;

		fr = f_read(&selectedSong.wavFile[0], target->dest_sound, target->size, &br);
		size_t zeros = target->size - br;

		if(zeros)
		{
			memset(target->dest_sound + br, 0, zeros);
		}
//		else
//			[[likely]] sound_strech(target);

		// чтение данных из файла click и запись в буффер
		br = 0;

		fr = f_read(&selectedSong.wavFile[1], target->dest_click, target->size, &br);
		zeros = target->size - br;
		if (zeros)
		{
			memset(target->dest_click + br, 0, zeros);
		}
//		else
//			[[likely]] click_strech(target);

		selectedSong.read_chunk_count++;
	}

	void get_wav_name(emb_string &dir_path, emb_string &file_path, emb_string &mask)
	{
		DIR dir; /* Directory search object */
		FILINFO fno; /* File information */

		fr = f_findfirst(&dir, &fno, dir_path.c_str(), mask.c_str()); /* Start to search for photo files */
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

	void find(emb_string &dir_path, emb_string &sound_path, emb_string &click_path)
	{
		emb_string mask = "1_*.wav";
		get_wav_name(dir_path, sound_path, mask);
		if (!sound_path.empty())
		{
			mask = "2_*.wav";
			get_wav_name(dir_path, click_path, mask);
		}
	}

	//-------------------------------------------------------------------

	void action(action_param_t val, uint8_t play_index);
	void curr();
	void next();
	void prev();
	void writeSongContent(emb_string& songPath, emb_string& path1, emb_string& path2);
	//------------------------------------------------------------------------

	inline void notify(const query_notify_t &val)
	{
		if (cortex_isr_num())
		{
			// send comand from ISR
			BaseType_t xHigherPriorityTaskWoken;
			NotifyFromISR(*((uint32_t*) &val), eSetValueWithOverwrite,
					&xHigherPriorityTaskWoken);
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}
		else
		{
			// thread mode
			Notify(*((uint32_t*) &val), eSetValueWithOverwrite);
		}
	}

//	inline void sound_strech(const target_t *target)
//	{
//		for (auto &item : stretch[0])
//			if ((selectedSong.read_chunk_count % item.index == 0) && item.index)
//			{
//				int32_t diff = item.pos_diff * sizeof(wav_sample_t);
//
//				f_lseek(&selectedSong.wavFile[0], f_tell(&selectedSong.wavFile[0]) + diff);
//#if 0
//    	    	   if ( item.pos_diff < -1 )
//    	    	     {
//
//    	    		     auto s = reinterpret_cast< uint32_t* >(target->dest_sound) ;
//
//    	    		     uint32_t fill = s [ target->size / sizeof(wav_sample_t) - item.pos_diff ] ;
//
//                         std::for_each ( s + target->size / sizeof(wav_sample_t) - (item.pos_diff - 1) , s + target->size / sizeof(wav_sample_t) - 1 ,
//                        		 [ & ]( auto & i) { i = fill ; }
//                                       ) ;
//    	    	     }
//#endif
//			}
//	}
//
//	inline void click_strech(const target_t *target)
//	{
//		for (auto &item : stretch[1])
//			if ((selectedSong.read_chunk_count % item.index == 0) && item.index)
//			{
//				int32_t diff = item.pos_diff * sizeof(wav_sample_t);
//
//				f_lseek(&selectedSong.wavFile[1], f_tell(&selectedSong.wavFile[1]) + diff);
//#if 0
//    	    	   if ( item.pos_diff < -1 )
//    	    	     {
//
//    	    		     auto s = reinterpret_cast< uint32_t* >(target->dest_click) ;
//
//    	    		     uint32_t fill = s [ target->size / sizeof(wav_sample_t) - item.pos_diff ] ;
//
//                         std::for_each ( s + target->size / sizeof(wav_sample_t) - (item.pos_diff - 1) , s + target->size / sizeof(wav_sample_t) - 1 ,
//                        		 [ & ]( auto & i) { i = fill ; }
//                                       ) ;
//    	    	     }
//#endif
//			}
//	}

private:
	void Code();
	const target_t *target;


	FATFS fs; // дескриптор раздела

	FRESULT fr; // результат файловой операции
	UINT *fw;

//	struct stretch_t
//	{
//		size_t index;
//		int32_t pos_diff;
//	} __attribute__((packed))
//#ifndef __STRETCH_DATA_FILE__
//	static constexpr
//#endif
//	stretch[2][3] =
//	{
//	{
//	{ 5, -2 },
//	{ 71, -2 },
//	{ 0, 0 } },
//	{
//	{ 5, -2 },
//	{ 71, -2 },
//	{ 0, 0 } } };

	browser_t browser;


};

extern TFsStreamTask *FsStreamTask;

#endif /*__FS_STREAM_H__*/
