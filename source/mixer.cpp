#include <nds.h>
#include <fat.h>
#include <stdio.h>
#include <stdlib.h>

#include <maxmod9.h>

#include "ticklesds.h"
#include "mixer.h"

// mixer stack
mixerEntry mixerStack [ maxPoly ];
int numPlaying;

int inMixerCycleNow = 0;

//allocate mixer buffer to maximum size
s16 sampleBuffer[streamBufferLen];
s32 mixBuffer[streamBufferLen];

mm_ds_system sys;
mm_stream stream;


int initAudio()
{
	//----------------------------------------------------------------
	// initialize maxmod without any soundbank (unusual setup)
	//----------------------------------------------------------------
	
	sys.mod_count 			= 0;
	sys.samp_count			= 0;
	sys.mem_bank			= 0;
	sys.fifo_channel		= FIFO_MAXMOD;
	mmInit( &sys );
    
    // setup mixer stack
	for (int i=0; i<maxPoly; i++) {
		mixerStack[i].active = -1;
	}
	numPlaying = 0;
	
	stream.sampling_rate =  outputSampleRate;        // outputSampleRate
    stream.buffer_length = streamBufferLen;          // should be adequate
    stream.callback = fill_stream;       			// give fill routine
    stream.format = MM_STREAM_16BIT_MONO; 		// 16-bit mono
    stream.timer = MM_TIMER0;            		// use timer0
    stream.manual = 0;                   		// auto filling
    
    mmStreamOpen( &stream );
	
	return 0;
}

mm_word fill_stream( mm_word length, mm_addr dest, mm_stream_formats format )
{
    inMixerCycleNow = 1;
	
	s16 *target = (s16 *)dest;	// set pointer to output buffer
	
	//reset mix buffer
	unsigned int i;
	
	for (i=0; i<length; i++) {
		mixBuffer[i]=0;
	}
	

	for (int entry=0; entry<maxPoly; entry++) {
		if (mixerStack[entry].active==1) {
			if (mixerStack[entry].sample->isPlaying()==1) {
				mixerStack[entry].sample->getSampleBlock(sampleBuffer, (int)length);
				
				for(i=0; i<(int)length; i++){
					mixBuffer[i]+=sampleBuffer[i];
				}
			}
		}
	
	}

	for(i=0; i<(int)length; i++ )
	{
		*target++ = (s16)mixBuffer[i];
		
	}
	
    inMixerCycleNow = 0;
	
	return length;
}

int initSampler()
{

	fatInitDefault();
	
	return 0;
}


int mixerMaint()
{

	// clean out empty entries
	int tempIndex=0;
	while (inMixerCycleNow==1) {;};
	
	for (int i=0; i<maxPoly; i++) {
		if (mixerStack[i].active==1) {
			tempIndex++;
			if (mixerStack[i].sample->isPlaying()!=1) {
				mixerStack[i].active=0;
				tempIndex--;
			}
		
		}
	
	}

	return tempIndex;
}

int stopAll()
{
	
	while (inMixerCycleNow==1) {;};
	
	for (int i=0; i<maxPoly; i++) {
		if (mixerStack[i].active==1) {
			if (mixerStack[i].sample->isPlaying()==1) {
				mixerStack[i].sample->stop();
				mixerStack[i].active=0;
				
			}
		
		}
	
	}
	
	return 0;
	
}


int startSample(void * samplePtr, int bank, int index)
{
	int empty=-1, entry=0, alreadyPlaying=0;
	
	//iprintf("starting sample...\n");
	
	while (inMixerCycleNow==1) {;};
	
	for (entry=0; entry<maxPoly; entry++) {
		if (mixerStack[entry].active==1){
			if (mixerStack[entry].sample==samplePtr){
				alreadyPlaying=1;
				//iprintf("sample already playing/restart...\n");
				(Csample *)mixerStack[entry].sample->restart();
			}
		}
	}	
	
	if (alreadyPlaying==0) {
		entry=0;
		while(empty==-1 && entry<maxPoly) {
			if (mixerStack[entry].active!=1) {
				empty=1;
			} else {entry++;}
		}
	
		if(empty==-1) {entry=0;}	// if no empty slots, reclaim first slot

		
		if ( ((Csample *)samplePtr)->isLoaded()==1 ){
			//iprintf("starting sample/new...\n");
			mixerStack[entry].sample=(Csample *)samplePtr;
			mixerStack[entry].active=1;
			mixerStack[entry].sample->play();
		}
	
	}
	return 0;
}




int getNumPlaying()
{
	return numPlaying;
}