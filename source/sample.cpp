/*
	sample.cpp
	------------
	Csample class functions
*/

#include <nds.h>
#include <fat.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/dir.h>
#include <maxmod9.h>

#include "ticklesds.h"
#include "sample.h"

s16 tempBuffer[streamBufferLen];
int inMixerCycle = 0;

int Csample::init( char * sFilename, int srate)
{
	
	while (inMixerCycle==1) {;};
	
	//iprintf("loading %s\n",sFilename);
	fp = fopen (sFilename, "rb");
	RIFFChunk fileheader;
	FormatChunk header;
	DataChunk dataheader;

	//bufferLen = buffsize;

	playbackRate = srate;
	
	// preinitialize some variables
	
	loaded = -1;
	playing = -1;
	bufferEnd = -1;
	bufferIndex = 0;
	filename = sFilename;
	
	if (strlen(title)<1) {
		sprintf(title, "%s",sFilename);
		//title = sFilename;		// change later with setTitle()
	}
	
	playmode = pm_default;	// change later with setPlayMode()
	

	fread(&fileheader,sizeof(fileheader), 1, fp);
	fread(&header,sizeof(header), 1, fp);
	fread(&dataheader,sizeof(dataheader), 1, fp);

	if(fileheader.chunkID!=0x46464952) {
		iprintf("Not a RIFF WAV!\n");
		return (-1);
	}

	if(fileheader.riffType!=0x45564157) {
		iprintf("Not a WAVE!\n");
		return (-1);
	}

	if(header.chunkID!=0x20746d66) {
		iprintf("Not a fmt chunk!\n");
		return (-1);
	}

	if(header.wFormatTag!=1) {
		iprintf("Not an uncompressed wave!\n");
		return (-1);
	}

	if(header.wChannels!=1) {
		iprintf("Not a mono wave! %d\n", header.wChannels);
		return (-1);
	}


	if(header.dwSamplesPerSec!=16000 && header.dwSamplesPerSec!=32000) {
		iprintf("\n\n\n\n\n\nNot a 16000Hz or 32000Hz wave! %d\n", (int)header.dwSamplesPerSec);
		return (-1);
	}

		
	if(header.wBitsPerSample!=16) {
		iprintf("Not a 16 bit wave!\n");
		return (-1);
	}

	int found_PCM_data=-1;
	int eof_wave=-1;
	int status=0;
	
	while(!found_PCM_data && !eof_wave){

		if(dataheader.chunkID!=0x61746164 && dataheader.chunkID!=0x64617461) {
			iprintf(" ChunkID = %s\n",(char *)dataheader.chunkID);
			fseek(fp,dataheader.chunkSize,SEEK_CUR);
			status=fread(&dataheader, sizeof(dataheader), 1, fp);
			if (status<(int)sizeof(dataheader)) eof_wave=1;
		} else found_PCM_data=1;
	}

	if (!found_PCM_data) {
		iprintf("No waveform data in file!\n");
		return (-1);
	}
	
	audioStart = ftell( fp );
	
	// find audio length
	fseek(fp,0,SEEK_END);
	sampleLength =  ftell( fp );
	sampleLength -= audioStart;
	
	sampleRate = (int)header.dwSamplesPerSec;
	channels = header.wChannels;
	bufferLen = diskBlocks*streamBufferLen*2*channels;		// in bytes
	
	//iprintf ("  Wave start at %d\n    buffersize: %d   sample rate: %d\n", audioStart, bufferLen, sampleRate);
	
	// try to alloc buffer mem
	
	if (readBuffer==NULL) {
		readBuffer = (s16 *) malloc ( bufferLen );		// in words
	}
	ringBuffer.threshBuf[0]=bufferLen/2;
	ringBuffer.threshBuf[1]=bufferLen;	
	ringBuffer.startPtr[0]=readBuffer;
	ringBuffer.startPtr[1]=(readBuffer+(bufferLen/4));
	
	if (readBuffer==NULL) {
			loaded = 0;
		} else {
			loaded = 1;
		}
		
	return 0;
}


Csample::~Csample()
{
	while (inMixerCycle==1) {;};
	
	if ( readBuffer != NULL ) free( readBuffer );
	
}


int Csample::play()
{
	
	while (inMixerCycle==1) {;};
	
	if(loaded) {
	
		
		restart();
		
		
	} else { return (-1); }

	return 0;
}


int Csample::readSample(int buffer)
{

	int readLength = bufferLen/2;
	s16 *dest = ringBuffer.startPtr[buffer];
	
	if (bufferIndex==-1) {
		fseek(fp, audioStart, SEEK_SET);	// reset to beginning of audio	
	}

	int bytesRead = fread(dest, 1, readLength, fp);

	int moreRead = readLength - bytesRead;
	if ( moreRead>0 ) {
		dest+=(bytesRead/2);	
		if (playmode==pm_loop || playmode==pm_mommentary){
			fseek(fp, audioStart, SEEK_SET);		// reset to start of audio
			fread(dest, 1, moreRead, fp);
		} else {
			memset(dest, 0, moreRead);
			bufferEnd=1;
		}
	}	

	if (bufferIndex==-1 && bufferEnd!=1) {
		dest = ringBuffer.startPtr[1];
		bytesRead = fread(dest, 1, readLength, fp);

		moreRead = readLength - bytesRead;
		if ( moreRead>0 ) {
			dest+=(bytesRead/2);	
			if (playmode==pm_loop || playmode==pm_mommentary){
				fseek(fp, audioStart, SEEK_SET);		// reset to start of audio
				fread(dest, 1, moreRead, fp);
			} else {
				memset(dest, 0, moreRead);
				bufferEnd=1;
			}
		}	
	}



	return 0;
}


int Csample::getSampleBlock(void * ptr, int length)
{

	inMixerCycle=1;
	
	s16 *target = (s16 *)ptr;
	s16 *src = (readBuffer+(bufferIndex/2));

	if (bufferIndex==-1) {
		readSample(0);
		bufferIndex=0;
	}
	
	if (sampleRate==16000) {
		for (int i=0; i<length; i+=2) {
			*target++=(*src>>1);
			*target++=(*src++>>1);
		}
		
		bufferIndex+=(length*channels);
	
	} else {
	
		for (int i=0; i<length; i++) {
			*target++=(*src++>>1);
		}

		bufferIndex+=(length*2*channels);
	}
	
	inMixerCycle=0;
	
	if ( bufferIndex>=ringBuffer.threshBuf[ringBuffer.currentBuf] ) {
		
		if(bufferEnd==1) {
			playing = -1;
		} else {
			readSample(ringBuffer.currentBuf);	
		}
		
		if (ringBuffer.currentBuf==1) {
			bufferIndex=0;
			ringBuffer.currentBuf=0;
		} else {
			ringBuffer.currentBuf=1;
		}
	
	}

	return 0;

}



int Csample::restart()
{
	
	while (inMixerCycle==1) {;};
	
	playing = 0;
	
	//iprintf("Csample::restart()...\n");

	bufferEnd = -1;
	bufferIndex = -1;
	ringBuffer.currentBuf=0;
	playing = 1;
	
	return 0;
}



int Csample::isLoaded()
{
	return loaded;
}

char * Csample::getTitle()
{
	return title;
}


int Csample::setTitle( char * newTitle)
{

	char * temp = title;
	
	
	
	/*while (*newTitle!=0) {
		*temp++=*newTitle++;
	}
	
	*temp=0;*/
	
	sprintf(title, "%s",newTitle);
	iprintf("setTitle: %s\n",title);

	return 0;

}




int Csample::isPlaying()
{
	return playing;
}


int Csample::stop()
{
	
	while (inMixerCycle==1) {;};

	playing = 0;
	return 0;
}

int Csample::close()
{

	while (inMixerCycle==1) {;};
	
	playing = -1;
	loaded = -1;
	if ( fp != NULL ) fclose (fp);
	
	return 0;

}




