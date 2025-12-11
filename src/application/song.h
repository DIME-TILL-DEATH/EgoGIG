#ifndef SRC_APPLICATION_SONG_H_
#define SRC_APPLICATION_SONG_H_

#include "appdefs.h"

class Song
{
public:
	emb_string trackPath[4];
	emb_string trackName[4];
	uint8_t trackCount{0};
};


#endif /* SRC_APPLICATION_SONG_H_ */
