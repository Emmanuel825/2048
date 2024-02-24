/*
 * game.h
 *
 *  Created on: 21 janv. 2024
 *      Author: emman
 */

#include <SDL2/SDL.h>
#define RIGHT	1
#define DOWN	2
#define LEFT	3
#define UP		4
#define NB_SAVES 7

typedef struct s_game t_game;


t_game *GameNew(int size, int cellBorderColor, int cellColor, SDL_Rect sceneRect);
void GameInit(t_game *pGame);
void GameDel(t_game *pGame);
void GameDraw(t_game *pGame, int margin, int cellSize, SDL_Renderer *pRenderer);
void GameContinue(t_game *pGame);
int GameRun(t_game *pGame, int dir);

void GameStepBack(t_game *pGame);
void GamePrint(t_game *pGame);
