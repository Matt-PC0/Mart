/*
 * these are versions of functions in raylib
 */
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "compat.h"
#include "state.h"

bool Is_Mouse_Pressed( SDL_MouseButtonFlags button_mask)
{
    SDL_MouseButtonFlags buttons = SDL_GetMouseState(NULL, NULL);
    return (buttons & button_mask) && !(last_frame_mouse_flags & button_mask);
}
bool Is_Mouse_Released( SDL_MouseButtonFlags button_mask)
{
    SDL_MouseButtonFlags buttons = SDL_GetMouseState(NULL, NULL);
    return !(buttons & button_mask) && (last_frame_mouse_flags & button_mask);
}
bool Is_Mouse_Down( SDL_MouseButtonFlags button_mask)
{
    SDL_MouseButtonFlags buttons = SDL_GetMouseState(NULL, NULL);
    return (buttons & button_mask);
}
bool Is_Mouse_UP( SDL_MouseButtonFlags button_mask)
{
    SDL_MouseButtonFlags buttons = SDL_GetMouseState(NULL, NULL);
    return !(buttons & button_mask);
}

void Fill_Rect(SDL_Renderer *renderer, SDL_FRect rect, SDL_Color color)
{
    if (color.a == 0xff)
    {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer, &rect);
    }
    else
    {
        SDL_Surface *pixel = SDL_CreateSurface(1,1, SDL_PIXELFORMAT_RGBA8888);
        SDL_FillSurfaceRect(pixel, NULL, SDL_MapRGBA(SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA8888), NULL, color.r,color.g,color.b,color.a));
        SDL_Texture *pixel_texture = SDL_CreateTextureFromSurface(renderer, pixel);
        SDL_SetTextureBlendMode(pixel_texture, SDL_BLENDMODE_BLEND);
        SDL_DestroySurface(pixel);

        SDL_RenderTexture(renderer, pixel_texture, NULL, &rect);

        SDL_DestroyTexture(pixel_texture);
    }
}
void Draw_Rect(SDL_Renderer *renderer, SDL_FRect rect, SDL_Color color, float thickness)
{
    if (thickness <= 0 || thickness > rect.w || thickness > rect.h)
    {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderRect(renderer, &rect);
    }
    else
    {
        //top
        Fill_Rect(renderer, (SDL_FRect){rect.x, rect.y, rect.w, thickness}, color);
        //left
        Fill_Rect(renderer, (SDL_FRect){rect.x, rect.y + thickness, thickness, rect.h - thickness*2}, color);
        //right
        Fill_Rect(renderer, (SDL_FRect){rect.x+rect.w - thickness, rect.y + thickness, thickness, rect.h - thickness*2}, color);
        //bottom
        Fill_Rect(renderer, (SDL_FRect){rect.x, rect.y+rect.h - thickness, rect.w, thickness}, color);
    }
}

/*
  I dont like doing this, having to load a texture every frame hurts my soul.
*/
void Draw_Text( TTF_Font *font, SDL_FPoint pos, float height, const char *text, SDL_Color fg, SDL_Color bg)
{
    SDL_Surface *text_surface = TTF_RenderText_Shaded(font, text, 0, fg, bg);
    SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    float scale = height / text_texture->h;
    SDL_RenderTexture(renderer, text_texture, NULL, &(SDL_FRect){ pos.x, pos.y, text_texture->w * scale, text_texture->h * scale });
    SDL_DestroyTexture(text_texture);
    SDL_DestroySurface(text_surface);
}

double GetTime( void )
{
    double time_ms = SDL_GetTicks();
    return time_ms / 1000;
}

double Lerp(double start, double end, double amount)
{
    double result = start + amount*(end - start);

    return result;
}

bool Is_Window_Focused( SDL_Window *window )
{
    SDL_WindowFlags flags = SDL_GetWindowFlags(window);
    return (flags & SDL_WINDOW_MOUSE_FOCUS);
}
