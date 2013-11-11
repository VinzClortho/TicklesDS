/*
	sample.h
	------------
	defines Csample class
*/

#include <nds.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <maxmod9.h>

#include "ticklesds.h"

// playmode types
enum PlayModes { pm_default, pm_loop, pm_stopable, pm_mommentary, pm_velocity };


// WAVE header info
typedef struct {
	long          chunkID;
	long         	chunkSize;
  	long          riffType;
	} RIFFChunk;

typedef struct {
	long           chunkID;
	long           chunkSize;
  	short          wFormatTag;
  	unsigned short wChannels;
	unsigned long  dwSamplesPerSec;
	unsigned long  dwAvgBytesPerSec;
	unsigned short wBlockAlign;
	unsigned short wBitsPerSample;
	} FormatChunk;

typedef struct {
	long chunkID;
	unsigned long chunkSize;
	} DataChunk;


typedef struct {
	int currentBuf;
	s16 * startPtr[2];
	int threshBuf[2];
	} typeRingBuffer;



class Csample {
	
	
	private:
		static int numSamples;
	
		// status
		int initialized;
		int loaded;
		int playing;
		
		// wave info
		char * filename;
		int channels;
		int sampleRate, playbackRate;
		long sampleLength;	// in samples
		
		int audioStart;
	
		// sample info
		int playmode;
		char title [256];
	
		FILE* fp;
		int bufferLen;
		//s16 actualBuffer[diskBlock];
		s16 * readBuffer;
		int bufferEnd;
		int bufferIndex;
		typeRingBuffer ringBuffer;
		
	public:
		
		int init( char * sFilename, int srate);
		int play();
		int stop();
		int restart();
		int close();
		int setPlaymode(int iPlaymode);
		int getPlaymode();
		int getSampleBlock(void * ptr, int length);
		int isLoaded();
		int isPlaying();
		char * getTitle();
		int setTitle( char * newTitle);
		int readSample(int buffer);		
		~Csample();
		
		
};


		