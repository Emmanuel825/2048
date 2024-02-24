/*
 * app.h
 *
 *  Created on: 21 janv. 2024
 *      Author: emman
 */


#include <SDL2/SDL.h>
#define SHADE_GREY				190
#define APP_SIZE				500
#define APP_GAME_MARGIN			APP_SIZE*25/100
#define APP_GAME_SIZE			4
#define APP_GAME_CELL_SIZE		(APP_SIZE-(2*APP_GAME_MARGIN))/APP_GAME_SIZE
#define APP_GAME_SCENE_SIZE		APP_GAME_CELL_SIZE*APP_GAME_SIZE

typedef struct s_app t_app;

t_app *AppNew(char *title, int size, int colorBackground);
void AppDel(t_app *pApp);
void AppRun(t_app *pApp);
