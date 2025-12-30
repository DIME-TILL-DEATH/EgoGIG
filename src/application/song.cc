#include "song.h"

#include "fs_stream.h"
#include "init.h"

char Song::buf[FF_MAX_LFN + 4];

Song::FsError Song::load(const emb_string& songPath)
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

	f_gets(buf, FF_MAX_LFN, &songFile);
	trackPath[0] = buf;
	if(trackPath[0].size() < 5) trackPath[0].clear(); // *.wav - minimum 5 symbols

	f_gets(buf, FF_MAX_LFN, &songFile);
	trackPath[1] = buf;
	if(trackPath[1].size() < 5) trackPath[1].clear(); // *.wav - minimum 5 symbols
	f_close(&songFile);

	for(uint8_t i=0; i<Player::maxTrackCount; i++)
	{
		if (!trackPath[i].empty())
		{
			if ((fr = f_open(&wavFile[i], trackPath[i].c_str(), FA_READ)) != FR_OK)
				return eFsError;

			// проверка на валидность формата WAV файла 1
			if (!isValidWave(&wavFile[i], i))
			{
				f_close(&wavFile[i]);
				return eNotRiffWave;
			}
			else
			{
				soundDataOffset[i] = wavFile[i].fptr;
			}

			emb_string::size_type lineEndPos = trackPath[i].find_last_of('\n');
			if(lineEndPos != emb_string::npos)
			{
				trackPath[i] = trackPath[i].substr(0, lineEndPos);
			}

			trackName[i] = trackPath[i];
			trackName[i] = trackName[i].substr(0, trackPath[i].find(".wav"));
			trackName[i] = trackName[i].substr(5, trackName[i].length());
			trackName[i] = trackName[i].substr(trackName[i].find_last_of('/') + 1, trackName[i].length());
		}
	}

	// MAX FROM TRACKS
	m_songSize = trackSize[0] / 4 / 4410;

	temp_mid = trackPath[0];
	temp_mid = temp_mid.substr(0, trackPath[0].find(".wav"));
	temp_mid.append(".mid");

	midiPlayer.openMidiFile(temp_mid.c_str());

	return eOk;
}

Song::FsError Song::save(const emb_string& songPath)
{
	FIL file;
	FRESULT fr = f_open(&file, songPath.c_str(), FA_OPEN_ALWAYS | FA_WRITE);
	UINT fw;

	if(fr != FR_OK)	return FsError::eFsError;

	f_lseek(&file, 0);
	f_truncate(&file);

	char temp = 62;
	if (playNext)
		f_write(&file, &temp, 1, &fw);
	else
		f_lseek(&file, 0);

	for(uint8_t i=0; i<Player::maxTrackCount; i++)
	{
		if(trackPath[i].empty()) continue;

		emb_string finalString;
		emb_printf::sprintf(finalString, "%s\n", trackPath[i].c_str());
		f_puts(finalString.c_str(), &file);
	}

	f_close(&file);
	return eOk;
}

void Song::close()
{
	for(uint8_t i =0; i < Player::maxTrackCount; i++)
	{
		f_close(&wavFile[i]);
		f_close(&midiFile);
	}
}

bool Song::isValidWave(emb_string filePath)
{
	FRESULT fr;

	FIL songFile;
	fr = f_open(&songFile, filePath.c_str(), FA_READ);
	if (fr != FR_OK) return false;

	return isValidWave(&songFile, 0);
}

bool Song::isValidWave(FIL *file, uint8_t num)
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
//	volatile size_t po_data = 36;
	while (1)
	{
		f_read(file, data_id, 4, &br);

		if((data_id[0] == 'd' && data_id[1] == 'a' && data_id[2] == 't' && data_id[3] == 'a'))
		{
			f_read(file, &sz, 4, &br);
			break;
		}

//		po_data = f_tell(file) - 3;
//		f_lseek(file, po_data);
	}

	trackSize[num] = sz;

	// Обработка какого-то необычного типа файлов?

//	if (!num)
//	{
//		if ((po_data % 6))
//		{
//			f_lseek(file, f_tell(file) + 2);
////			swap_need |= (swap_need_t) snSound;
//		}
//		trackSize[0] = sz;
//	}
//
//	else
//	{
//		//if ( wh.subchunk1Size == 18 ) { f_lseek(file, f_tell(file)+2) ; swap_need |= (swap_need_t)snClick ;}
//		trackSize[1] = sz;
//	}

	return true;
}

emb_string Song::songName()
{
	return trackName[0];
}
