/****************************************************************
 * TicklesDS audio sampler
 ****************************************************************/
 
#include <nds.h>
#include <stdio.h>

#include "ticklesds.h"

#include "mixer.h"
#include "screens.h"

// prototypes

int initSampler();

// global variables


// for gfx debugging
//---------------------------------------------------------------------------------
void Vblank() {
//---------------------------------------------------------------------------------
	
}

int main(void) {

	//irqSet(IRQ_VBLANK, Vblank);

	// Init top and bottom video layers
	initVideo();
	
	// Init Maxmod audio
	initAudio();
	
	// Init file system and mixer
	initSampler();
	
	mainScreen();
	
	return 0;
	
}






