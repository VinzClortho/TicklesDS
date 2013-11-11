#include <nds.h>
#include <maxmod9.h>

#include "sample.h"

typedef struct {
	int active;
	int bank, index;
	Csample * sample;
	
	} mixerEntry;


int initAudio();
mm_word fill_stream( mm_word length, mm_addr dest, mm_stream_formats format );
int initSampler();
int mixerMaint();
int getNumPlaying();
int startSample(void * samplePtr, int bank, int index);
int stopAll();

