/*
 * game.c
 *
 *  Created on: 21 janv. 2024
 *      Author: emman
 */


#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "game.h"
#include <time.h>

#define mBitSet(f,m)	((f)|=(m))
#define mBitClear(f,m)	((f)&=(~m))
#define mBitMsk(f,m)	((f)&(m))
#define mBitToggle(f,m)	((f)^=(m))
#define mIsBitSet(f,m)	(((f)&(m))==(m))
#define mIsBitClear(f,m) ((~(f)&(m))==(m))

#define CELL_VALUE_MASK		0x0FFFFFFF
#define CELL_MERGED_MASK	0x80000000

#define SetCellMerged(c)	mBitSet(c, CELL_MERGED_MASK)
#define UnsetCellMerged(c)	mBitClear(c, CELL_MERGED_MASK)
#define IsCellMerged(c)		mIsBitSet(c, CELL_MERGED_MASK)
#define CellValue(c)		mBitMsk(c, CELL_VALUE_MASK)

#define mRGBA_r(rgba)	((Uint8)(rgba>>24))
#define mRGBA_g(rgba)	((Uint8)(rgba>>16))
#define mRGBA_b(rgba)	((Uint8)(rgba>>8))
#define mRGBA_a(rgba)	((Uint8)(rgba>>0))
#define mRGBA(r,g,b,a)	((Uint32)r<<24|(Uint32)g<<16|(Uint32)b<<8|(Uint32)a)

struct s_game{
	int *scene[NB_SAVES];
	int *sceneConfront;
	int *sceneCheck;
	int cellsColors[2][17];
	int cellVoid;
	int cellVoidSaved;
	int isWin;
	int cont;
	int size;
	int cellBorderColor;
	int cellColor;
	int nbSaves;
	SDL_Rect sceneRect;
	TTF_Font *font;
	SDL_Surface *cellVoidSurface, *valueSurface, *winSurface, *savesSurfaces;
	SDL_Texture *cellVoidTexture, *valueTexture, *winTexture, *savesTexture;
};

int _GameMove(t_game *pGame, int dir, int isChecking);
void _GameMoveRight(t_game *pGame);
void _GameMoveDown(t_game *pGame);
void _GameMoveLeft(t_game *pGame);
void _GameMoveUp(t_game *pGame);
void _GameSceneSave(t_game *pGame, int *scene1, int *scene2);
int _GameCheck(t_game *pGame);


t_game *GameNew(int size, int cellBorderColor, int cellColor, SDL_Rect sceneRect){
	t_game *pGame = (t_game*)malloc(sizeof(t_game));
	for(int i = 0 ; i<NB_SAVES+1 ; i++)pGame->scene[i] = (int*)malloc((size*size)*sizeof(int));
	pGame->sceneConfront = (int*)malloc((size*size)*sizeof(int));
	pGame->sceneCheck = (int*)malloc((size*size)*sizeof(int));
	pGame->sceneRect = sceneRect;
	pGame->isWin = 0;
	pGame->cont = 0;
	pGame->nbSaves = 0;
	pGame->size = size;
	pGame->cellVoid = size*size;
//	printf("%d", size);
//	printf("%d", pGame->size);
	pGame->cellBorderColor = cellBorderColor;
	pGame->cellColor = cellColor;
	for (int i = 0; i < NB_SAVES; ++i) {
		for(int j = 0 ; j<pGame->cellVoid ; j++){
			pGame->scene[i][j]=0;
			pGame->sceneConfront[i]=0;
		}
	}
	for(int j = 0 ; j<pGame->cellVoid ; j++){
		pGame->sceneConfront[j]=0;
	}

	TTF_Init();

	pGame->font=TTF_OpenFont("arial.ttf", 200);
	if(pGame->font==NULL){
		fprintf(stderr, "failed to open font : %s", TTF_GetError());
		return NULL;
	}
	return pGame;
}

void GameDel(t_game *pGame){
	for(int i = 0 ; i<NB_SAVES ; i++)free(pGame->scene[i]);
	free(pGame->sceneCheck);
	free(pGame->sceneConfront);
	free(pGame);
}

void GameInit(t_game *pGame){
	srand((unsigned int)time(NULL));
		int k, l, q;
		for(int i = 0 ; i < 2 ; i++){
			do{
				k = rand()%pGame->size;
				l = rand()%pGame->size;
			}while(pGame->scene[0][k*pGame->size+l]!=0);
			q = rand()%10;
			if(q == 4)pGame->scene[0][k*pGame->size+l]=4;
			else pGame->scene[0][k*pGame->size+l]=2;
		}
}

void GameDraw(t_game *pGame, int margin, int cellSize, SDL_Renderer *pRenderer){
	SDL_Rect	cell, cellVoidRect=(SDL_Rect){0,0,50,20}, nbSavesRect=(SDL_Rect){pGame->sceneRect.w/2, 10, 50, 20};
	char value[5], cellVoidValue[2], saves[1];
	for(int i = 0; i < pGame->size; i++){
		for(int j = 0 ; j < pGame->size ; j++){
			SDL_SetRenderDrawColor(pRenderer, mRGBA_r(pGame->cellBorderColor), mRGBA_g(pGame->cellBorderColor), mRGBA_b(pGame->cellBorderColor), mRGBA_a(pGame->cellBorderColor));
			SDL_RenderDrawRect(pRenderer, &(SDL_Rect){(margin+(i*cellSize)),(margin+(j*cellSize)),cellSize,cellSize});
			if(CellValue(pGame->scene[0][i*pGame->size+j])>0){
				cell = (SDL_Rect){(margin+(j*cellSize)+1),(margin+(i*cellSize)+1),cellSize-2,cellSize-2};
				SDL_SetRenderDrawColor(pRenderer, 0,0,255,0);
				SDL_RenderFillRect(pRenderer, &cell);
				sprintf(value, "%d", CellValue(pGame->scene[0][i*pGame->size+j]));
				pGame->valueSurface=TTF_RenderText_Blended(pGame->font, value, (SDL_Color){255,255,255,255});
				pGame->valueTexture=SDL_CreateTextureFromSurface(pRenderer, pGame->valueSurface);
				SDL_RenderCopy(pRenderer, pGame->valueTexture, NULL, &cell);
				sprintf(cellVoidValue, "%d", pGame->cellVoid);
				pGame->cellVoidSurface=TTF_RenderText_Blended(pGame->font, cellVoidValue, (SDL_Color){0,0,0,0});
				pGame->cellVoidTexture=SDL_CreateTextureFromSurface(pRenderer, pGame->cellVoidSurface);
				SDL_RenderCopy(pRenderer, pGame->cellVoidTexture, NULL, &cellVoidRect);
				sprintf(saves, "%d", pGame->nbSaves);
//				sprintf(saves, "%d", sizeof(t_game));
				pGame->savesSurfaces=TTF_RenderText_Blended(pGame->font, saves, (SDL_Color){0,0,0,0});
				pGame->savesTexture=SDL_CreateTextureFromSurface(pRenderer, pGame->savesSurfaces);
				SDL_RenderCopy(pRenderer, pGame->savesTexture, NULL, &nbSavesRect);
			}
		}
	}
	if(pGame->isWin == 1 && pGame->cont==0){
		pGame->winSurface=TTF_RenderText_Blended(pGame->font, "GAGNE", (SDL_Color){0,0,0,0});
		pGame->winTexture=SDL_CreateTextureFromSurface(pRenderer, pGame->winSurface);
		SDL_RenderCopy(pRenderer, pGame->winTexture, NULL, &pGame->sceneRect);
	}
}

int GameRun(t_game *pGame, int dir){
	srand((unsigned int)time(NULL));
	int k, l, q;
	if(pGame->isWin == 0 || pGame->cont == 1){
		if(_GameMove(pGame, dir, 0)){
			do{
				k = rand()%pGame->size;
				l = rand()%pGame->size;
			}while(pGame->scene[0][k*pGame->size+l]!=0);
			q = rand()%10;
			if(q == 4)pGame->scene[0][k*pGame->size+l]=4;
			else pGame->scene[0][k*pGame->size+l]=2;
			pGame->cellVoid=pGame->size*pGame->size;
			for(int i=0; i<(pGame->size*pGame->size) ; i++){
				UnsetCellMerged(pGame->scene[0][i]);
				if(CellValue(pGame->scene[0][i])>0)pGame->cellVoid--;
			}
			if(pGame->cellVoid == 0){
				if(!_GameCheck(pGame))return 1;
			}
			else if(_GameCheck(pGame)==2)pGame->isWin=1;

		}
	}
	return 0;
}

void GameContinue(t_game *pGame){
	if(pGame->isWin)pGame->cont = 1;
}

int _GameMove(t_game *pGame, int dir, int isChecking){
	_GameSceneSave(pGame, pGame->scene[0], pGame->sceneConfront);
	switch (dir) {
		case RIGHT:
			_GameMoveRight(pGame);
			break;
		case DOWN:
			_GameMoveDown(pGame);
			break;
		case LEFT:
			_GameMoveLeft(pGame);
			break;
		case UP:
			_GameMoveUp(pGame);
			break;
		default:
			break;

	}

	for(int i = 0 ; i<(pGame->size*pGame->size) ; i++){
		if(pGame->scene[0][i]!=pGame->sceneConfront[i]){
			if(!isChecking){
				for(int i = NB_SAVES ; i > 1 ; i--){
					_GameSceneSave(pGame, pGame->scene[i-1], pGame->scene[i]);
				}
				_GameSceneSave(pGame, pGame->sceneConfront, pGame->scene[1]);
				if(pGame->nbSaves < NB_SAVES-1) pGame->nbSaves++;
			}
			return 1;
		}
	}
	_GameSceneSave(pGame, pGame->sceneConfront, pGame->scene[0]);
	return 0;
//	printf("\n%d\n", pGame->cellVoid);

}

void GameStepBack(t_game *pGame){
	if(pGame->nbSaves>0){
		for(int i = 0 ; i < pGame->nbSaves-1 ; i++){
			_GameSceneSave(pGame, pGame->scene[i+1], pGame->scene[i]);
		}
		pGame->nbSaves--;
	}
}

void _GameMoveRight(t_game *pGame){
	//répète l'opération autant de fois qu'il y a de colonnes
	for(int j = pGame->size-1 ; j >= 0 ; j--){
		//commence de la colonne la plus à droite et va jusqu'à la colonne précédant celle la plus à gauche
		for (int i = pGame->size-1 ; i > 0; i--) {
			//parcours toutes les lignes avant de passer à la colonne suivante
			for(int k = 0 ; k < pGame->size ; k++){
				//vérifie que la case à gauche vaut la case actuelle ou zéro
				if(CellValue(pGame->scene[0][k*pGame->size+i]) == 0 || CellValue(pGame->scene[0][k*pGame->size+i]) == CellValue(pGame->scene[0][k*pGame->size+(i-1)])){
					//vérifie que la case actuelle vaut zéro ou qu'elle n'a pas déjà été additionnée avec une autre
					if(CellValue(pGame->scene[0][k*pGame->size+i])==0 || (!IsCellMerged(pGame->scene[0][k*pGame->size+(i-1)]) && !IsCellMerged(pGame->scene[0][k*pGame->size+(i)]))){
						//Si la case actuelle ne vaut pas zéro, indique qu'elle vient d'être additionnée
						if(CellValue(pGame->scene[0][k*pGame->size+i]) != 0)SetCellMerged(pGame->scene[0][k*pGame->size+i]);
						//additionne la case actuelle avec la case sur la gauche
						pGame->scene[0][k*pGame->size+i] += CellValue(pGame->scene[0][k*pGame->size+(i-1)]);
						//met la case sur la gauche à zéro
						pGame->scene[0][k*pGame->size+(i-1)] = 0;
					}
				}
			}
		}
	}
}

void _GameMoveLeft(t_game *pGame){
	for(int j = pGame->size-1 ; j >= 0 ; j--){
		for (int i = 0; i < pGame->size-1; i++) {
			for(int k = 0 ; k < pGame->size ; k++){
				if(CellValue(pGame->scene[0][k*pGame->size+i]) == 0 || CellValue(pGame->scene[0][k*pGame->size+i]) == CellValue(pGame->scene[0][k*pGame->size+(i+1)])){
					if(CellValue(pGame->scene[0][k*pGame->size+i])==0 || (!IsCellMerged(pGame->scene[0][k*pGame->size+(i+1)]) && !IsCellMerged(pGame->scene[0][k*pGame->size+(i)]))){
						if(CellValue(pGame->scene[0][k*pGame->size+i]) != 0){
							printf("merged\n");
							SetCellMerged(pGame->scene[0][k*pGame->size+i]);
						}
						pGame->scene[0][k*pGame->size+i] += CellValue(pGame->scene[0][k*pGame->size+(i+1)]);
						pGame->scene[0][k*pGame->size+(i+1)] = 0;
					}
				}
			}
		}
	}
}
void _GameMoveDown(t_game *pGame){
	for(int j = pGame->size-1 ; j >= 0 ; j--){
		for (int i = 0; i < pGame->size; i++) {
			for(int k = pGame->size-1 ; k > 0; k--){
				if(CellValue(pGame->scene[0][k*pGame->size+i]) == 0 || CellValue(pGame->scene[0][k*pGame->size+i]) == CellValue(pGame->scene[0][(k-1)*pGame->size+i])){
					if(CellValue(pGame->scene[0][k*pGame->size+i])==0 || (!IsCellMerged(pGame->scene[0][(k-1)*pGame->size+i]) && !IsCellMerged(pGame->scene[0][k*pGame->size+(i)]))){
						if(CellValue(pGame->scene[0][k*pGame->size+i]) != 0){
							printf("merged\n");
							SetCellMerged(pGame->scene[0][k*pGame->size+i]);
						}
						pGame->scene[0][k*pGame->size+i] += CellValue(pGame->scene[0][(k-1)*pGame->size+i]);
						pGame->scene[0][(k-1)*pGame->size+i] = 0;
					}
				}
			}
		}
	}
}
void _GameMoveUp(t_game *pGame){
	for(int j = pGame->size-1 ; j >= 0 ; j--){
		for (int i = 0; i < pGame->size; i++) {
			for(int k = 0 ; k < pGame->size-1; k++){
				if(CellValue(pGame->scene[0][k*pGame->size+i]) == 0 || CellValue(pGame->scene[0][k*pGame->size+i]) == CellValue(pGame->scene[0][(k+1)*pGame->size+i])){
//					if(CellValue(pGame->scene[0][k*pGame->size+i]) != 0)pGame->cellVoid++;
					if(CellValue(pGame->scene[0][k*pGame->size+i])==0 || (!IsCellMerged(pGame->scene[0][(k+1)*pGame->size+i]) && !IsCellMerged(pGame->scene[0][k*pGame->size+(i)]))){
						if(CellValue(pGame->scene[0][k*pGame->size+i]) != 0){
							printf("merged\n");
							SetCellMerged(pGame->scene[0][k*pGame->size+i]);
						}
						pGame->scene[0][k*pGame->size+i] += CellValue(pGame->scene[0][(k+1)*pGame->size+i]);
						pGame->scene[0][(k+1)*pGame->size+i] = 0;
					}
				}
			}
		}
	}
}

void _GameSceneSave(t_game *pGame, int *scene1, int *scene2){
	for(int i = 0 ; i < (pGame->size*pGame->size) ; i++){
		scene2[i]=scene1[i];
	}

}

int _GameCheck(t_game *pGame){
	if(pGame->cellVoid == 0){
		_GameSceneSave(pGame, pGame->scene[0], pGame->sceneCheck);
		for(int i=LEFT ; i<=UP ; i++){
			if(_GameMove(pGame, i, 1)){
				_GameSceneSave(pGame, pGame->sceneCheck, pGame->scene[0]);
				return 1;
			}
			_GameSceneSave(pGame, pGame->sceneCheck, pGame->scene[0]);
		}
		printf("perdu\n");
	}
	else{
		for(int i = 0 ; i<(pGame->size*pGame->size) ; i++){
			if(pGame->scene[0][i]==2048)return 2;
		}
	}
	return 0;
}

//void GamePrint(t_game *pGame){
//	for(int i = 0 ; i < pGame->size ; i++){
//		for(int k = 0 ; k < pGame->size ; k++){
//			printf("%d\t", CellValue(pGame->scene[0][i*pGame->size+k]));
//		}
//		printf("\n");
//	}
//	printf("\n");
////	printf("\n%d\n", pGame->cellVoid);
//}
