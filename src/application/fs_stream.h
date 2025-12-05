#ifndef __FS_STREAM_H__
#define __FS_STREAM_H__

#include "appdefs.h"
#include "ff.h"
#include "sd_storage.h"
#include <vector>

#include "midi_stream.h"
#include "midi_player.h"

class TFsStreamTask: public TTask
{
public:

	enum swap_need_t
	{
		snNo = 0, snSound = 1, snClick = 2, snAll = 3
	};

	typedef enum
	{
		eOk = 0, eFsError, eNotRiffWave, // формат wave_a не соответсвует RIFF/WAV
		eWaveNotPresent,  // file_sound ("1_*.wav") отсутсвует
		eNotMidi,
	} error_t;

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

	inline TFsStreamTask(const char *name, const int stack_size,
			const int priority) :
			TTask(name, stack_size, priority, true)
	{

	}

	enum notify_t
	{
		qn_data = 0, qn_next, qn_prev, qn_curr, qn_owner, qn_action
	};
	enum action_param_t
	{
		ap_1_wav, ap_2_wav
	};
	struct query_notify_t
	{
		notify_t notify :8;
		union
		{
			struct
			{
				action_param_t action_param :16;
				uint32_t play_index :8;
				uint32_t play_next :8;
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
	inline void action_notify(action_param_t val, uint8_t play_index,
			uint8_t play_next)
	{
		query_notify_t qn;
		qn.notify = qn_action;
		qn.action_param = val;
		qn.play_index = play_index;
		qn.play_next = play_next;
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

	emb_string dir;
	emb_string song;
	emb_string sound_path;
	emb_string click_path;

	inline error_t open_song_name(size_t index, emb_string &dst, uint8_t mode,
			uint8_t num)
	{
		FIL f_temp;
		emb_printf::sprintf(dst, "%s/%1.ego", browser.play_list_folder.c_str(),
				index);
		fr = f_open(&f_temp, dst.c_str(), FA_READ);
		if (fr != FR_OK)
			return eWaveNotPresent;
		uint8_t temp;
		f_read(&f_temp, &temp, 1, fw);
		if (temp == 62)
			f_lseek(&f_temp, 1);
		else
			f_lseek(&f_temp, 0);
		f_gets(browser.buf, FF_MAX_SS, &f_temp);
		if (num)
			f_gets(browser.buf, FF_MAX_SS, &f_temp);
		f_close(&f_temp);
		dst = browser.buf;
		if (dst.empty())
			return eWaveNotPresent;
		if (!mode)
			dst = dst.substr(dst.find_last_of('/') + 1, dst.length());
		dst.erase(dst.find(".wav"), 5);
		return eOk;
	}

	inline error_t delete_track(size_t index)
	{
		emb_string temp;
		emb_printf::sprintf(temp, "%s/%1.ego", browser.play_list_folder.c_str(),
				index);
		f_unlink(temp.c_str());
		return eOk;
	}

	inline error_t open(size_t index) // выбор
	{
		read_chunk_count = 0;

		close();
		emb_string temp_mid;

		emb_printf::sprintf(song, "%s/%1.ego", browser.play_list_folder.c_str(),
				index);

		fr = f_open(&file_song, song.c_str(), FA_READ);
		if (fr != FR_OK)
			return eWaveNotPresent;

		uint8_t temp;
		f_read(&file_song, &temp, 1, fw);
		if (temp == 62)
		{
			f_lseek(&file_song, 1);
			next_play = 1;
		}
		else
		{
			f_lseek(&file_song, 0);
			next_play = 0;
		}
		f_gets(browser.buf, FF_MAX_SS, &file_song);
		sound_path = browser.buf;
		f_gets(browser.buf, FF_MAX_SS, &file_song);
		click_path = browser.buf;
		f_close(&file_song);

		swap_need = snNo;

		if (!sound_path.empty())
		{
			if ((fr = f_open(&file_sound, sound_path.c_str(), FA_READ))
					!= FR_OK)
				return eFsError;
			// проверка на валидность формата WAV файла A
			if (!is_valid_wave(&file_sound, 0))
			{
				f_close(&file_sound);
				return eNotRiffWave;
			}
			else
				sound_data_offset = file_sound.fptr;
			// чтение имени файла sound
			name = sound_path;
			name = name.substr(0, sound_path.find(".wav"));
			name = name.substr(5, name.length());

			midi_player.midi_stream.clear();
			temp_mid = sound_path;
			temp_mid = temp_mid.substr(0, sound_path.find(".wav"));
			temp_mid.append(".mid");
			if (f_open(&file_click, temp_mid.c_str(), FA_READ) == FR_OK)
			{
				if (f_size(&file_click) < 4096)
				{
					f_close(&file_click);
					// midi
					float parse_file(const char *path,
							midi_stream_t &midi_stream);
					float sf = parse_file(temp_mid.c_str(),
							midi_player.midi_stream);

					midi_player.midi_stream.sort_and_merge();
					for (auto &v : midi_player.midi_stream.items)
					{
						v.time_tics = (v.time_tics * 44100) / sf;
					}
					midi_player.reset();
				}
				else
					f_close(&file_click);
			}
		}
		else
		{
			// отсутсвует файл wave
			return eWaveNotPresent;
		}

		if (!click_path.empty())
		{
			if ((fr = f_open(&file_click, click_path.c_str(), FA_READ))
					!= FR_OK)
				return eFsError;
			// проверка на валидность формата WAV файла B
			if (!is_valid_wave(&file_click, 1))
			{
				f_close(&file_click);
				return eNotRiffWave;
			}
			else
				click_data_offset = file_click.fptr;
		}

		/*
		 // midi
		 midi_player.midi_stream.clear() ;
		 float parse_file(const char *path, midi_stream_t& midi_stream) ;
		 float sf = parse_file("/SONGS/AMT_DEMO_TRACKS/LEHMANIZED-T1.mid", midi_player.midi_stream);

		 midi_player.midi_stream.sort_and_merge();
		 for ( auto & v : midi_player.midi_stream.items  )
		 {
		 v.time_tics = (v.time_tics * 44100) / sf ;
		 }
		 midi_player.reset();
		 */

		return eOk;
	}

	inline uint8_t next_pl(void)
	{
		return next_play;
	}

	inline void curr_path(emb_string &dst)
	{
		f_getcwd(browser.buf, FF_MAX_LFN);
		dst = emb_string(browser.buf);
	}

	inline size_t pos()
	{
		return (f_tell(&file_sound) - sound_data_offset) / sizeof(wav_sample_t);
	}

	inline void pos(size_t val)
	{
		fr = f_lseek(&file_sound,
				val * sizeof(wav_sample_t) + sound_data_offset);
		fr = f_lseek(&file_click,
				val * sizeof(wav_sample_t) + click_data_offset);

		midi_player.pos(val);

	}

	inline void close()
	{
		f_close(&file_sound);
		f_close(&file_click);
		name.clear();
		size = 0;
		fr = FR_OK;
	}

	inline void sound_name(emb_string &dst, bool fullname = false) const
	{
		dst = name;
		if (!fullname)
			dst = dst.substr(dst.find_last_of('/') + 1, dst.length());
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

	inline bool eof(void)
	{
		//if ( is_valid_ff_object(&file_click) )
		//return f_eof(&file_sound) &&  f_eof(&file_click) ;
		//else
		return f_eof(&file_sound);
	}

	inline bool file_flag(void)
	{
		if (browser.fno.fattrib & AM_DIR)
			return false;
		else
			return true;
	}

	inline uint32_t sound_size()
	{
		uint32_t temp = size / 4 / 4410;
		return temp;
	}

	inline uint32_t click_size()
	{
		uint32_t temp = size1 / 4;
		return temp;
	}
	void enter_dir(const char *name, const char *high_level_node, bool begin);

	inline bool SoundNeedSwap()
	{
		return swap_need & snSound;
	}
	inline bool ClickNeedSwap()
	{
		return swap_need & snClick;
	}

	inline void MidiEventProcess()
	{
		midi_player.process();
	}

protected:

	inline bool mount()
	{
		// монтирование раздела
		fr = f_mount(&fs, "SD", 1);
		return fr == FR_OK;
	}

	inline bool umount()
	{
		close();

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

	bool is_valid_wave(FIL *file, uint8_t num);

	inline void get()
	{
		// чтение данных из файла wave и запись в буффер
		size_t br = 0;
		extern volatile uint32_t samp_point;
		samp_point = file_sound.fptr - sound_data_offset;
		fr = f_read(&file_sound, target->dest_sound, target->size, &br);
		size_t zeros = target->size - br;

		if (zeros)
		{
			memset(target->dest_sound + br, 0, zeros);
		}
		else
			[[likely]] sound_strech(target);

		// чтение данных из файла click и запись в буффер
		br = 0;
		if ((file_click.fptr) > samp_point)
			samp_point = file_click.fptr;
		fr = f_read(&file_click, target->dest_click, target->size, &br);
		zeros = target->size - br;
		if (zeros)
		{
			memset(target->dest_click + br, 0, zeros);
		}
		else
			[[likely]] click_strech(target);

		read_chunk_count++;
	}

	void get_wav_name(emb_string &dir_path, emb_string &file_path,
			emb_string &mask)
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

	void find(emb_string &dir_path, emb_string &sound_path,
			emb_string &click_path)
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

	void action(action_param_t val, uint8_t play_index, uint8_t play_next);
	void curr();
	void next();
	void prev();
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

	inline void sound_strech(const target_t *target)
	{
		for (auto &item : stretch[0])
			if ((read_chunk_count % item.index == 0) && item.index)
			{
				int32_t diff = item.pos_diff * sizeof(wav_sample_t);

				f_lseek(&file_sound, f_tell(&file_sound) + diff);
#if 0
    	    	   if ( item.pos_diff < -1 )
    	    	     {

    	    		     auto s = reinterpret_cast< uint32_t* >(target->dest_sound) ;

    	    		     uint32_t fill = s [ target->size / sizeof(wav_sample_t) - item.pos_diff ] ;

                         std::for_each ( s + target->size / sizeof(wav_sample_t) - (item.pos_diff - 1) , s + target->size / sizeof(wav_sample_t) - 1 ,
                        		 [ & ]( auto & i) { i = fill ; }
                                       ) ;
    	    	     }
#endif
			}
	}

	inline void click_strech(const target_t *target)
	{
		for (auto &item : stretch[1])
			if ((read_chunk_count % item.index == 0) && item.index)
			{
				int32_t diff = item.pos_diff * sizeof(wav_sample_t);

				f_lseek(&file_click, f_tell(&file_click) + diff);
#if 0
    	    	   if ( item.pos_diff < -1 )
    	    	     {

    	    		     auto s = reinterpret_cast< uint32_t* >(target->dest_click) ;

    	    		     uint32_t fill = s [ target->size / sizeof(wav_sample_t) - item.pos_diff ] ;

                         std::for_each ( s + target->size / sizeof(wav_sample_t) - (item.pos_diff - 1) , s + target->size / sizeof(wav_sample_t) - 1 ,
                        		 [ & ]( auto & i) { i = fill ; }
                                       ) ;
    	    	     }
#endif
			}
	}

private:
	void Code();
	const target_t *target;
	emb_string name; // имя потока (содержится в файле /xx/name.txt )

	FATFS fs; // дескриптор раздела
	FIL file_song; // дескриптор файла 'song'
	FIL file_sound; // дескриптор файла 'sound'
	FIL file_click; // дескриптор файла 'click'
	FRESULT fr; // результат файловой операции
	UINT *fw;

	size_t size; // размер наибольшего из двух потоков
	size_t size1; // размер 2 track
	size_t sound_data_offset;
	size_t click_data_offset;

	uint32_t read_chunk_count;
	struct stretch_t
	{
		size_t index;
		int32_t pos_diff;
	} __attribute__((packed))
#ifndef __STRETCH_DATA_FILE__
	static constexpr
#endif
	stretch[2][3] =
	{
	{
	{ 5, -2 },
	{ 71, -2 },
	{ 0, 0 } },
	{
	{ 5, -2 },
	{ 71, -2 },
	{ 0, 0 } } };

	uint8_t next_play;

	browser_t browser;

	uint32_t swap_need;

	midi_player_t midi_player;

};

extern TFsStreamTask *FsStreamTask;

#endif /*__FS_STREAM_H__*/
