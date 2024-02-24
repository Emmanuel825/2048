/*
 ============================================================================
 Name        : 2048.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-styl
 ============================================================================
 */

#include <stdio.h>
//#include <stdlib.h>
#include <SDL2/SDL.h>
#include "app.h"

#define mRGBA(r,g,b,a)	((Uint32)r<<24|(Uint32)g<<16|(Uint32)b<<8|(Uint32)a)

int main(int argc, char **argv) {

	t_app *pApp = AppNew("2048", 50, mRGBA(SHADE_GREY, SHADE_GREY, SHADE_GREY, SHADE_GREY));
	AppRun(pApp);
	AppDel(pApp);

	return EXIT_SUCCESS;
}
