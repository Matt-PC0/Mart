#ifndef _COMPAT_H_
#define _COMPAT_H_

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

bool Is_Mouse_Pressed( SDL_MouseButtonFlags button_mask);
bool Is_Mouse_Released( SDL_MouseButtonFlags button_mask);
bool Is_Mouse_Down( SDL_MouseButtonFlags button_mask);
bool Is_Mouse_UP( SDL_MouseButtonFlags button_mask);

void Fill_Rect(SDL_Renderer *renderer, SDL_FRect rect, SDL_Color color);
void Draw_Rect(SDL_Renderer *renderer, SDL_FRect rect, SDL_Color color, float thickness);

void Draw_Text( TTF_Font *font, SDL_FPoint pos, float height, const char *text, SDL_Color fg, SDL_Color bg);

double GetTime( void );

double Lerp(double start, double end, double amount);

bool Is_Window_Focused( SDL_Window *window );

#endif //ifndef _COMPAT_H_
