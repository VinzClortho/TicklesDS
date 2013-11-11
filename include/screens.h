#include <nds.h>
#include <stdio.h>

int initVideo();
int loadsaveScreen(int saveOrLoad);
int mainScreen();
int waitForTouch();
int bgFadeIn(int bgImage);
int bgFadeOut();

int loadConfig(char *s);