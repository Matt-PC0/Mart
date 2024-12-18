#ifndef _About_Main_H_
#define _About_Main_H_

#include <SDL3/SDL.h>

SDL_AppResult About_Init(void);
SDL_AppResult About_Event(void *appstate, SDL_Event *event);
SDL_AppResult About_Iterate(void *appstate);
void About_Quit(void *appstate, SDL_AppResult result);

#endif //ifndef _About_Main_H_
