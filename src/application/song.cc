#include "song.h"

//#include "fs_stream.h"
char Song::buf[FF_MAX_LFN + 4];

uint8_t Song::load(const emb_string& songPath)
{
	FRESULT fr;
	UINT *fw;
	read_chunk_count = 0;

	close();
	emb_string temp_mid;

	FIL songFile;
	fr = f_open(&songFile, songPath.c_str(), FA_READ);
	if (fr != FR_OK)
		return eSongFileNotFound;

	uint8_t temp;
	f_read(&songFile, &temp, 1, fw);
	if(temp == 62)
	{
		f_lseek(&songFile, 1);
		playNext = 1;
	}
	else
	{
		f_lseek(&songFile, 0);
		playNext = 0;
	}

	f_gets(buf, FF_MAX_SS, &songFile);
	trackPath[0] = buf;
	f_gets(buf, FF_MAX_SS, &songFile);
	trackPath[1] = buf;
	f_close(&songFile);

	if (!trackPath[0].empty())
	{
		if ((fr = f_open(&wavFile[0], trackPath[0].c_str(), FA_READ)) != FR_OK)
			return eFsError;

		// проверка на валидность формата WAV файла 1
		if (!is_valid_wave(&wavFile[0], 0))
		{
			f_close(&wavFile[0]);
			return eNotRiffWave;
		}
		else
		{
			soundDataOffset[0] = wavFile[0].fptr;
		}

		// чтение имени файла sound
		trackName[0] = trackPath[0];
		trackName[0] = trackName[0].substr(0, trackPath[0].find(".wav"));
		trackName[0] = trackName[0].substr(5, trackName[0].length());
		trackName[0] = trackName[0].substr(trackName[0].find_last_of('/') + 1, trackName[0].length());

		// Load midi
//		midi_player.midi_stream.clear();
//		temp_mid = trackPath[0];
//		temp_mid = temp_mid.substr(0, trackPath[0].find(".wav"));
//		temp_mid.append(".mid");
//
//		FIL file_midi;
//		if (f_open(&midiFile, temp_mid.c_str(), FA_READ) == FR_OK)
//		{
//			if (f_size(&midiFile) < 4096)
//			{
//				f_close(&midiFile);
//				// midi
//				float parse_file(const char *path, midi_stream_t &midi_stream);
//				float sf = parse_file(temp_mid.c_str(), midi_player.midi_stream);
//
//				midi_player.midi_stream.sort_and_merge();
//				for (auto &v : midi_player.midi_stream.items)
//				{
//					v.time_tics = (v.time_tics * 44100) / sf;
//				}
//				midi_player.reset();
//			}
//			else
//				f_close(&midiFile);
//		}
	}
	else
	{
		// отсутсвует файл wave
		return eWaveNotPresent;
	}

	if (!trackPath[1].empty())
	{
		if ((fr = f_open(&wavFile[1], trackPath[1].c_str(), FA_READ))
				!= FR_OK)
			return eFsError;
		// проверка на валидность формата WAV файла B
		if (!is_valid_wave(&wavFile[1], 1))
		{
			f_close(&wavFile[1]);
			return eNotRiffWave;
		}
		else
			soundDataOffset[1] = wavFile[1].fptr;
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

void Song::close()
{
	for(uint8_t i =0; i < maxTrackCount; i++)
	{
		f_close(&wavFile[i]);
		f_close(&midiFile);
	}
}

bool Song::is_valid_wave(FIL *file, uint8_t num)
{
	FRESULT fr;
	UINT br;

	wave_header_t wh;
	if ((fr = f_read(file, &wh, 12, &br)) != FR_OK)
		return false;

	// RIFF
	if (!(wh.chunkId[0] == 'R' && wh.chunkId[1] == 'I' && wh.chunkId[2] == 'F'
			&& wh.chunkId[3] == 'F'))
	{
		return false;
	}

	// WAVE
	if (!(wh.format[0] == 'W' && wh.format[1] == 'A' && wh.format[2] == 'V'
			&& wh.format[3] == 'E'))
	{
		return false;
	}

	uint32_t find_data = file->fptr;
	char data_id[4];
	while (1)
	{
		f_lseek(file, find_data++);
		f_read(file, data_id, 4, &br);
		if ((data_id[0] == 'f' && data_id[1] == 'm' && data_id[2] == 't'
				&& data_id[3] == ' '))
		{
			f_read(file, &wh.subchunk1Size, 20, &br);
			break;
		}
	}

	if (wh.subchunk1Size != 16 && wh.subchunk1Size != 18)
		return false;
	if (wh.audioFormat != 1)
		return false;
	if (wh.numChannels != 2)
		return false;
	if (wh.sampleRate != 44100)
		return false;
	if (wh.bitsPerSample != 16)
		return false;

	size_t sz;
	volatile size_t po_data = 36;
	while (1)
	{

		f_read(file, data_id, 4, &br);

		if ((data_id[0] == 'd' && data_id[1] == 'a' && data_id[2] == 't'
				&& data_id[3] == 'a'))
		{
			f_read(file, &sz, 4, &br);
			break;
		}

		po_data = f_tell(file) - 3;
		f_lseek(file, po_data);
	}

	if (!num)
	{
		if ((po_data % 6))
		{
			f_lseek(file, f_tell(file) + 2);
//			swap_need |= (swap_need_t) snSound;
		}
		trackSize[0] = sz;
	}

	else
	{
		//if ( wh.subchunk1Size == 18 ) { f_lseek(file, f_tell(file)+2) ; swap_need |= (swap_need_t)snClick ;}
		trackSize[1] = sz;
	}

	return true;
}

emb_string Song::songName()
{
	return trackName[0];
}
