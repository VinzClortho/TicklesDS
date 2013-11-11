#include <nds.h>
#include <stdio.h>
#include <string>

#include "ticklesds.h"

#include "screens.h"
#include "mixer.h"

// gfx includes
#include "transceiver.h"
#include "controllerscreen.h"
#include "textscreen1.h"
#include "nextbutton.h"
#include "prevbutton.h"

#define maxBanks			16				
#define samplesPerBank		8

using std::string;

PrintConsole topScreen;
PrintConsole bottomScreen;
int topgfxbg, bottomgfxbg;

int loadedSamples = 0;
int currentScreen = -1;
int currentBank = 0;
int lastBank = 0;
string bankNames [maxBanks];

int keyMap[samplesPerBank] = {KEY_UP, KEY_LEFT, KEY_RIGHT, KEY_DOWN, KEY_X, KEY_Y, KEY_A, KEY_B };

int iMajor = majorVer;
int iMinor = minorVer;
int iSub = subVer;


enum Screens { _loadsaveScreen, _mainScreen };
enum Buttons { buttonUP, buttonLT, buttonRT, buttonDN, buttonX, buttonY, buttonA, buttonB };

Csample sampleSlot[maxBanks][samplesPerBank];

class button {
	public:
		int top, left;
		int width, height;
		void *funcPtr;
		void *imagePtr;

};

int initVideo()
{

	videoSetMode( MODE_5_2D );
	videoSetModeSub( MODE_5_2D );
	
	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankB(VRAM_B_MAIN_BG);
	vramSetBankC(VRAM_C_SUB_BG);
	
	topgfxbg = bgInit(3, BgType_Bmp8, BgSize_B8_256x256, 4, 0);
	bottomgfxbg = bgInitSub(3, BgType_Bmp8, BgSize_B8_256x256, 4, 0);
	
	// palette is Gimp default
	dmaCopy(transceiverPal, BG_PALETTE, 256*2);	// add transceiver palette
	dmaCopy(transceiverPal, BG_PALETTE_SUB, 256*2);	// add textscreen1 palette

	
	// set bgs to black (12 is black in gimp default pal)
	memset (bgGetGfxPtr(topgfxbg), 12, 256*192);
	memset (bgGetGfxPtr(bottomgfxbg), 12, 256*192);

	consoleInit(&topScreen, 1, BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, true);
	consoleInit(&bottomScreen, 1,BgType_Text4bpp, BgSize_T_256x256, 31, 1, false, true);


	consoleSelect(&topScreen);
	consoleClear();
	consoleSelect(&bottomScreen);
	consoleClear();

	return 0;
}


int bgFadeIn(int bgImage)
{

	bgMosaicEnable(topgfxbg);
	bgMosaicEnable(bottomgfxbg);
	
	bgSetMosaic( 15, 15 );
	
	switch (bgImage) {
		case _loadsaveScreen:
			dmaCopy(transceiverBitmap,  bgGetGfxPtr(topgfxbg), 256*192);
			memset(bgGetGfxPtr(bottomgfxbg), 0,  256*192);
			break;
		case _mainScreen:
			dmaCopy(controllerscreenBitmap,  bgGetGfxPtr(topgfxbg), 256*192);
			dmaCopy(textscreen1Bitmap,  bgGetGfxPtr(bottomgfxbg), 256*192);
		default:
			break;
	}
	

	int mosaicVal=15;
	
	while( (mosaicVal--)>0 ) {
	
		swiWaitForVBlank();
		swiWaitForVBlank();
		
		bgSetMosaic( mosaicVal, mosaicVal );
	}

	return 0;

}

int bgFadeOut()
{

	int mosaicVal=0;
	
	bgSetMosaic( 0, 0 );
	
	while( (mosaicVal++)<15 ) {
		swiWaitForVBlank();
		swiWaitForVBlank();
		
		bgSetMosaic( mosaicVal, mosaicVal );
	}


	bgMosaicDisable(topgfxbg);
	bgMosaicDisable(bottomgfxbg);
	
	return 0;
}


int loadsaveScreen(int saveOrLoad)
{

	if (currentScreen!=_loadsaveScreen) {
		
		bgFadeIn(_loadsaveScreen);
		
		consoleSelect(&topScreen);
		
		iprintf("\x1b[14;3HTicklesDS v%d,%d,%d\n",iMajor, iMinor, iSub);	
		
		currentScreen=_loadsaveScreen;
	}

	if (saveOrLoad==0) {
	
		iprintf("\x1b[16;3HLoading config...\n");	
		consoleSelect(&bottomScreen);
		
		// reset bank names
		for (int i=0;i<maxBanks;i++) {
			bankNames[i]="";
		}
		
		//load config file here...
		
		loadConfig("fat1:/tickles/tickles-config");
		
		/*sampleSlot[0][0].init( (char *)"fat1:/wav/01-Treatpause.wav", outputSampleRate);
		sampleSlot[0][0].setTitle( (char *)"TreatPause");
		sampleSlot[0][1].init( (char *)"fat1:/wav/02-Spacehits.wav", outputSampleRate);
		sampleSlot[0][1].setTitle( (char *)"SpaceHits");
		sampleSlot[0][2].init( (char *)"fat1:/wav/03-Areyoureceivingme.wav", outputSampleRate);
		sampleSlot[0][2].setTitle( (char *)"AreYou");
		sampleSlot[0][3].init( (char *)"fat1:/wav/04-Treat_organ06.wav", outputSampleRate);
		sampleSlot[0][3].setTitle( (char *)"TreatOrgan1");
		sampleSlot[0][4].init( (char *)"fat1:/wav/05-Treat_mega1.wav", outputSampleRate);
		sampleSlot[0][4].setTitle( (char *)"KeysArpeg");
		sampleSlot[0][5].init( (char *)"fat1:/wav/06-Jaws_monologue.wav", outputSampleRate);
		sampleSlot[0][5].setTitle( (char *)"Jaws");
		sampleSlot[0][6].init( (char *)"fat1:/wav/07-Globaltreatbit2.wav", outputSampleRate);
		sampleSlot[0][6].setTitle( (char *)"GuitLoop");
		sampleSlot[0][7].init( (char *)"fat1:/wav/08-Treat_organ07.wav", outputSampleRate);
		sampleSlot[0][7].setTitle( (char *)"DoomOrgan");*/
	
		
		
		consoleSelect(&topScreen);
		iprintf("\x1b[22;4HTouch screen to start...\n");	
		waitForTouch();
		
		bgFadeOut();
		
		consoleSelect(&topScreen);
		consoleClear();
		consoleSelect(&bottomScreen);
		consoleClear();
		
	} 
	
	if (saveOrLoad==1) {
		iprintf("\x1b[16;3HSaving config... \n");	
		consoleSelect(&bottomScreen);
		
		consoleSelect(&topScreen);
	}

	return 0;

}

int mainScreen()
{
	
	touchPosition touchXY;	
	
	
	loadsaveScreen(0);			// do initial setup load
	
	//button prevButton, nextButton;
	
	//prevButton.imagePtr = &prevbuttonBitmap;
	
	while (1) {

		swiWaitForVBlank();

		// show current loaded samples in bank
		consoleSelect(&topScreen);

		iprintf("\x1b[17;3HBank: %d %s                 \n", currentBank, bankNames[currentBank].c_str());
		
		sampleSlot[currentBank][buttonUP].isLoaded()==1 ? iprintf("\x1b[18;1HUP: %9s          \n", sampleSlot[currentBank][buttonUP].getTitle()) : iprintf("\x1b[18;1HUP:            \n");
		sampleSlot[currentBank][buttonX].isLoaded()==1 ? iprintf("\x1b[18;17HX: %10s          \n",  sampleSlot[currentBank][buttonX].getTitle()) : iprintf("\x1b[18;17HX:            \n");
		sampleSlot[currentBank][buttonLT].isLoaded()==1 ? iprintf("\x1b[19;1HLT: %9s          \n", sampleSlot[currentBank][buttonLT].getTitle()) : iprintf("\x1b[19;1HLT:            \n");
		sampleSlot[currentBank][buttonY].isLoaded()==1 ? iprintf("\x1b[19;17HY: %10s          \n",  sampleSlot[currentBank][buttonY].getTitle()) : iprintf("\x1b[19;17HY:            \n");
		sampleSlot[currentBank][buttonRT].isLoaded()==1 ? iprintf("\x1b[20;1HRT: %9s          \n", sampleSlot[currentBank][buttonRT].getTitle()) : iprintf("\x1b[20;1HRT:            \n");
		sampleSlot[currentBank][buttonA].isLoaded()==1 ? iprintf("\x1b[20;17HA: %10s          \n",  sampleSlot[currentBank][buttonA].getTitle()) : iprintf("\x1b[20;17HA:            \n");
		sampleSlot[currentBank][buttonDN].isLoaded()==1 ? iprintf("\x1b[21;1HDN: %9s          \n", sampleSlot[currentBank][buttonDN].getTitle()) : iprintf("\x1b[21;1HDN:            \n");
		sampleSlot[currentBank][buttonB].isLoaded()==1 ? iprintf("\x1b[21;17HB: %10s          \n",  sampleSlot[currentBank][buttonB].getTitle()) : iprintf("\x1b[21;17HB:            \n");
	
		iprintf("\x1b[5;11H%d playing  \n", mixerMaint());

		consoleSelect(&bottomScreen);

		iprintf("\x1b[17;10H%d samples\n", loadedSamples);
		
		int keys = 0;

		// read input
		touchRead(&touchXY);
		scanKeys();
		keys = keysDown();

		// redraw BGs if coming from another screen
		if (currentScreen!=_mainScreen) {
			bgFadeIn(_mainScreen);
			currentScreen=_mainScreen;
		}
		
		// play triggered samples
		// *********************************************
		for (int i=0; i<samplesPerBank; i++) {
			if(keys & keyMap[i]) {
				if (sampleSlot[currentBank][i].isLoaded()==1) {
					startSample(&sampleSlot[currentBank][i],currentBank,i);
				}
			}
		}
		//************************************

		// Process touch screen input
		if (keys & KEY_TOUCH) {
			
			if (touchXY.px < leftThresh) {
				currentBank == 0 ? currentBank = maxBanks-1 : currentBank--;
			} else if (touchXY.px > rightThresh) {
				currentBank == maxBanks-1 ? currentBank=0 : currentBank++;
			} else if (touchXY.py < topThresh) {	
				stopAll();
			}
			
		}

		
		// print at using ansi escape sequence \x1b[line;columnH 
		//iprintf("\x1b[20;3HTouch x = %04X, %04X", touchXY.rawx, touchXY.px);
		//iprintf("\x1b[21;3HTouch y = %04X, %04X", touchXY.rawy, touchXY.py);
		//iprintf("\x1b[%01d;%01dHX\n", y, x);	
	}

	return 0;
}

int waitForTouch()
{
	int keys = 0;
	
	while( !(keys & KEY_TOUCH) ){
		scanKeys();
		keys = keysDown();
	}

	return 0;
}


/**********************************/
/* Sample configuration functions */
/**********************************/
int loadConfig(char *s)
{
	FILE *handle;
	char line_buffer[256];
	char work_buffer[256];
	int bank, sample, junk, i;

	handle=fopen(s,"r");

	if(handle==NULL) {
		iprintf("\x1b[22;4HCan't open config...\n");	
		waitForTouch();
		return (-1);
	}

	fscanf(handle, "%s", work_buffer);
	
	/* check to see if file is a tickles-config file */
	if( strcasecmp(work_buffer,"tickles-config" ) != 0) {
		iprintf("Wrong file type! %s\n",work_buffer);
		return (-1);
	}


	/* reset default sample info */
	for(bank=0;bank<maxBanks;bank++){
		for(sample=0;sample<samplesPerBank;sample++){
			sampleSlot[bank][sample].close();
		}
	}

	bank=0;
	sample=0;
	
	string sLine;
	char tempString[256];
	char tempString2[256];
	size_t found;
	
	
	while( !feof(handle) ) { 
		//fscanf(handle,"%s",work_buffer);
		fgets(work_buffer,256,handle);
		sLine = (work_buffer);
		
		//sscanf(work_buffer, "%s", tempString);
		
		
	
		/* sample bank position info */
		found=sLine.find("sample");
		if (found!=string::npos && int(found)<2) {
			sscanf(work_buffer,"sample %d,%d",&bank,&sample);
			//iprintf("Bank: %d  Sample: %d in config...\n",bank, sample);	
		}

		/* bank name info */
		found=sLine.find("bankname");
		if (found!=string::npos && int(found)<2) {
			sscanf(work_buffer,"bankname %d %s",&bank,&tempString2);
			if (bank<maxBanks && bank>-1) {
				bankNames[bank]=tempString2;
			}
			//iprintf("Bank %d: %s\n",bank, tempString2);	
			
		}
	

		/* get sample filename and initialize wave file */
		found=sLine.find("file");
		if (found!=string::npos && int(found)<2) {
			sscanf(work_buffer,"file %s",&tempString2);
			if (bank<maxBanks && sample<samplesPerBank)  {
				if (sampleSlot[bank][sample].init(tempString2, outputSampleRate)==0) {
					loadedSamples++;
				}
			}
		}
	

		/* get sample title */
		found=sLine.find("title");
		if (found!=string::npos && int(found)<2) {

			sscanf(work_buffer,"title %s",&tempString2);
			if (bank<maxBanks && sample<samplesPerBank)  {
				if (bank<maxBanks && sample<samplesPerBank) {
					sampleSlot[bank][sample].setTitle((char *)tempString2);
					//iprintf("sample title %s\n",tempString2);
				}
			}
		}



		/* get sample priority */
	//	if(strcasecmp(work_buffer, "priority")==0) {
	//		fscanf(handle,"%d",&junk);
	//		//samples[bank][sample].priority=junk;
	//	}

		/* get sample volume */
	//	if(strcasecmp(work_buffer, "volume")==0) {
	//		fscanf(handle,"%d",&junk);
	//		//samples[bank][sample].volume=junk;
	//	}

		/* get sample playmode */
	//	if(strcasecmp(work_buffer, "playmode")==0) {
	//		fscanf(handle,"%d",&junk);
	//		//samples[bank][sample].playmode=junk;
	//	}

		/* get MasterVolume */
    //	if(strcasecmp(work_buffer, "master-volume")==0) {
	//		fscanf(handle,"%d",&junk);
	//		//MasterVolume=junk;
	//	}


		/* manual end of config */
	//	if(strcasecmp(work_buffer, "end")==0) {
	//		fclose(handle);	
	//		return 0;
	//	}
	//else {	// skip to next line
	//	fgets(work_buffer,256,handle);
	//}


	}

	fclose(handle);	
	
	return 0;
}

