/*
 * app.c
 *
 *  Created on: 21 janv. 2024
 *      Author: emman
 */

#include <stdio.h>
//#include <stdlib.h>
#include "app.h"
#include "game.h"


#define mBitSet(f,m)	((f)|=(m))
#define mBitClear(f,m)	((f)&=(~m))
#define mBitToggle(f,m)	((f)^=(m))
#define mIsBitSet(f,m)	(((f)&(m))==(m))
#define mIsBitClear(f,m) ((~(f)&(m))==(m))
#define mRGBA_r(rgba)	((Uint8)(rgba>>24))
#define mRGBA_g(rgba)	((Uint8)(rgba>>16))
#define mRGBA_b(rgba)	((Uint8)(rgba>>8))
#define mRGBA_a(rgba)	((Uint8)(rgba>>0))
#define mRGBA(r,g,b,a)	((Uint32)r<<24|(Uint32)g<<16|(Uint32)b<<8|(Uint32)a<<0)



typedef enum e_enum{

	IS_ALL_SET			=	0xFFFFFFFF,
	IS_APP_INIT			=	0x80000000,
	IS_WINDOW_CREATE	=	0x08000000,
	IS_RENDER_CREATE	=	0x00800000,
	IS_GAME_CREATE		= 	0x00080000

}t_etat;

struct s_app{
	SDL_Window *pWindow;
	SDL_Renderer *pRenderer;
	SDL_Event event;
	t_etat etat;
	int colorBackground;
	t_game *pGame;
	FILE* log;
};

void _AppDraw(t_app *pApp);

t_app *AppNew(char *title, int size, int colorBackground){
	t_app *pApp = (t_app*)malloc(sizeof(t_app));
//	pApp->log = fopen("./log.txt", "a");
//	fprintf(pApp->log, "SDL init");
	mBitClear(pApp->etat, IS_ALL_SET);
	if((SDL_Init(SDL_INIT_VIDEO))<0){
		fprintf(stderr, "failed to init SDL : %s", SDL_GetError());
		return pApp;
	}
//	fprintf(pApp->log, "SDL init");
	mBitSet(pApp->etat, IS_APP_INIT);

	pApp->pWindow = SDL_CreateWindow("2048", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 500, 500, SDL_WINDOW_SHOWN);
	if(pApp->pWindow == NULL){
		fprintf(stderr, "failed to create window : %s", SDL_GetError());
		SDL_Quit();
		return pApp;
	}
	mBitSet(pApp->etat, IS_WINDOW_CREATE);

	pApp->pRenderer = SDL_CreateRenderer(pApp->pWindow, -1, SDL_RENDERER_ACCELERATED);
	if(pApp->pRenderer == NULL){
		fprintf(stderr, "failed to create accelerated renderer : %s", SDL_GetError());
		pApp->pRenderer = SDL_CreateRenderer(pApp->pWindow, -1, SDL_RENDERER_SOFTWARE);
		if(pApp->pWindow == NULL){
			fprintf(stderr, "failed to create software renderer : %s", SDL_GetError());
			SDL_DestroyWindow(pApp->pWindow);
			SDL_Quit();
			return pApp;
		}
	}
	mBitSet(pApp->etat, IS_RENDER_CREATE);
	pApp->pGame = GameNew(APP_GAME_SIZE, mRGBA(0,0,0,0), mRGBA(0,0,255,0), (SDL_Rect){APP_GAME_MARGIN, APP_GAME_MARGIN, APP_GAME_SCENE_SIZE, APP_GAME_SCENE_SIZE});
	if(pApp->pGame == NULL){
		fprintf(stderr, "failed to create game");
		SDL_DestroyRenderer(pApp->pRenderer);
		SDL_DestroyWindow(pApp->pWindow);
		SDL_Quit();
	}
	mBitSet(pApp->etat, IS_GAME_CREATE);
	pApp->colorBackground = colorBackground;
	printf("%d\n", sizeof(pApp->pGame));
//	int n=sizeof(pApp->pGame);
	return pApp;
}
void AppDel(t_app *pApp){


	if(mIsBitSet(pApp->etat, IS_GAME_CREATE))GameDel(pApp->pGame);
	if(mIsBitSet(pApp->etat, IS_RENDER_CREATE)){
		SDL_DestroyRenderer(pApp->pRenderer);
	}
//	fprintf(pApp->log, "renderer\n");
	if(mIsBitSet(pApp->etat, IS_WINDOW_CREATE))SDL_DestroyWindow(pApp->pWindow);
//	fclose(pApp->log);
	if(mIsBitSet(pApp->etat, IS_APP_INIT))SDL_Quit();
}

void AppRun(t_app *pApp){

	int quit = 0;
	printf("\n");
	GameInit(pApp->pGame);

	_AppDraw(pApp);
	while(!quit){
		if(SDL_PollEvent(&pApp->event)){
			switch (pApp->event.type) {
				case SDL_QUIT:
					quit = 1;
					break;
				case SDL_KEYUP:
					switch (pApp->event.key.keysym.sym) {
						case SDLK_ESCAPE:
							quit = 1;
							break;
						case SDLK_RIGHT:
							if(GameRun(pApp->pGame, RIGHT))quit = 1;
							_AppDraw(pApp);
							break;
						case SDLK_DOWN:
							if(GameRun(pApp->pGame, DOWN))quit = 1;
							_AppDraw(pApp);
							break;
						case SDLK_LEFT:
							if(GameRun(pApp->pGame, LEFT))quit = 1;
							_AppDraw(pApp);
							break;
						case SDLK_UP:
							if(GameRun(pApp->pGame, UP))quit = 1;
							_AppDraw(pApp);
							break;
						case SDLK_r:
							GameStepBack(pApp->pGame);
							_AppDraw(pApp);
							break;
						case SDLK_c:
							GameContinue(pApp->pGame);
							_AppDraw(pApp);
							break;
						default:
							break;
					}
					break;
				default:
					break;
			}
		}
	}

}

void _AppDraw(t_app *pApp){
	SDL_SetRenderDrawColor(pApp->pRenderer, mRGBA_r(pApp->colorBackground),
							mRGBA_g(pApp->colorBackground),
							mRGBA_b(pApp->colorBackground),
							mRGBA_a(pApp->colorBackground));
	SDL_RenderClear(pApp->pRenderer);
	GameDraw(pApp->pGame, APP_GAME_MARGIN, APP_GAME_CELL_SIZE, pApp->pRenderer);
	SDL_RenderPresent(pApp->pRenderer);
}
