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

	enum notify_t
	{
		qn_wav_data = 0,
		qn_midi_events,
		qn_next,
		qn_prev,
		qn_action
	};

	enum action_param_t
	{
		enter_directory,
		save_song
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

	TFsStreamTask(const char *name, const int stack_size, const int priority);

	inline void data_notify(const target_t *val)
	{
		target = val;

		query_notify_t qn =
		{ .notify = qn_wav_data };
		notify(qn);
	}

	inline void midi_notify(const uint64_t& start, const uint64_t& stop)
	{
		midiStartInterval = start;
		midiStopInterval = stop;

		query_notify_t qn =
		{ .notify = qn_midi_events };
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
			f_lseek(&selectedSong.wavFile[i], val * sizeof(wav_sample_t) + selectedSong.soundDataOffset[i]);

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
			FRESULT fr = f_getcwd(browser.buf, FF_MAX_SS);
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

	bool currentPathIsDirectory();
	void enter_dir(const char *name, const char *high_level_node, bool begin);
	inline void play_list_folder(emb_string &val) const
	{
		val = browser.play_list_folder.substr(
				browser.play_list_folder.find_last_of('/') + 1,
				browser.play_list_folder.length());
	}

	emb_string browserPlaylistFolder() const { return browser.play_list_folder; }

private:
	const target_t *target;
	uint64_t midiStartInterval;
	uint64_t midiStopInterval;

	FATFS fs; // дескриптор раздела
	UINT *fw;

	browser_t browser;

	void getWavData();
	void getMidiEvents();

	void getWavName(emb_string &dir_path, emb_string &file_path, emb_string &mask);

	//-------------------------------------------------------------------

	void action(action_param_t val, uint8_t play_index);
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

	void Code();
};

extern TFsStreamTask *FsStreamTask;

#endif /*__FS_STREAM_H__*/
