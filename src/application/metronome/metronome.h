#ifndef METRONOME_H_
#define METRONOME_H

#include "appdefs.h"
namespace Metronome
{

enum MetronomeType
{
	DEFAULT = 0,
	ABLETONE,
	CUBASE,
	MPC,
	PRO_TOOLS,
	WOOD
};

void setMetronomeType(MetronomeType type);
size_t dataSize();

extern const uint16_t* data;
}

#endif
