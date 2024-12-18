#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include "about_main.h"
#include "state.h"
#include "config.h"
#include "imgui.h"

const char *vl_gothic_licence[] = {
"Copyright (c) 1990-2003 Wada Laboratory, the University of Tokyo.",
"Copyright (c) 2003-2004 Electronic Font Open Laboratory (/efont/).",
"Copyright (C) 2003-2015 M+ FONTS PROJECT",
"Copyright (C) 2006-2015 Daisuke SUZUKI <daisuke@vinelinux.org>.",
"Copyright (C) 2006-2015 Project Vine <Vine@vinelinux.org>.",
"All rights reserved.",
"",
"Redistribution and use in source and binary forms, with or without",
"modification, are permitted provided that the following conditions",
"are met:",
"1. Redistributions of source code must retain the above copyright notice,",
"   this list of conditions and the following disclaimer.",
"2. Redistributions in binary form must reproduce the above copyright notice,",
"   this list of conditions and the following disclaimer in the documentation",
"   and/or other materials provided with the distribution.",
"3. Neither the name of the Wada Laboratory, the University of Tokyo nor",
"   the names of its contributors may be used to endorse or promote products",
"   derived from this software without specific prior written permission.",
"",
"THIS SOFTWARE IS PROVIDED BY WADA LABORATORY, THE UNIVERSITY OF TOKYO AND",
"CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT",
"NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A",
"PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE LABORATORY OR",
"CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,",
"EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,",
"PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;",
"OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,",
"WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR",
"OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF",
"ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.",
};
SDL_Texture *vl_gothic_licence_texture = NULL;
const char *mart_description = "Mart is a minimalist client for MPD focusing on displaying cover art as a compainion client\nMade by Matthew Conroy";
SDL_Texture *mart_description_texture = NULL;

SDL_Texture *logo_texture = NULL;
float about_scroll_y = 0;
float about_content_size = 0;
int about_w = 700;
int about_h = 700;

SDL_Surface *Render_Text_lines(const char *text[], int n_lines)
{
    SDL_Surface *surface = SDL_CreateSurface(0,0, SDL_PIXELFORMAT_RGBA8888);
    for (int i = 0; i < n_lines; i += 1)
    {
        const char *line = text[i];
        SDL_Surface *line_s = TTF_RenderText_Shaded(gui_state.font, (*line) ? line : " ", 0, (SDL_Color){0xFF,0xFF,0xFF,0xFF}, (SDL_Color){0});
        if (!line_s)
        {
            SDL_Log("Error: couldnt create line surface for multi-line text [%i]", i);
            About_Quit(NULL, SDL_APP_FAILURE);
        }

        int w = (line_s->w > surface->w) ? line_s->w : surface->w;
        int h = surface->h + line_s->h;
        SDL_Surface *new_s = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_RGBA8888);
        if (!new_s)
        {
            SDL_Log("Error: couldnt create surface copy for multi-line text");
            About_Quit(NULL, SDL_APP_FAILURE);
        }
        SDL_BlitSurface(surface, NULL, new_s, &(SDL_Rect){0,0, surface->w, surface->h});
        SDL_BlitSurface(line_s, NULL, new_s, &(SDL_Rect){0,surface->h, line_s->w, line_s->h});
        SDL_DestroySurface(surface);
        SDL_DestroySurface(line_s);
        surface = new_s;
    }
    return surface;
}

SDL_AppResult About_Init(void)
{
    about_scroll_y = 0;
    about_content_size = 0;
    SDL_CreateWindowAndRenderer("About Mart", about_w, about_h, 0, &about_window, &about_renderer);
    if (!about_window)
    {
        SDL_Log("ERROR: couldnt open popup window!\n");
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "couldnt open popup window\n", SDL_GetError(), NULL);
        return SDL_APP_FAILURE;
    }
    SDL_SetWindowIcon(about_window, logo_surface);
    if (!about_renderer)
    {
        SDL_Log("ERROR: couldnt create renderer for about popup!\n");
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "couldnt create renderer for about popup!\n", SDL_GetError(), NULL);
        return SDL_APP_FAILURE;
    }

    logo_texture = SDL_CreateTextureFromSurface(about_renderer, logo_surface);

    TTF_SetFontSize(gui_state.font, 25);

    SDL_Surface *mart_description_surface = TTF_RenderText_Shaded_Wrapped(gui_state.font, mart_description, 0, (SDL_Color){0xFF,0xFF,0xFF,0xFF}, (SDL_Color){0}, about_w);
    mart_description_texture = SDL_CreateTextureFromSurface(about_renderer, mart_description_surface);
    SDL_DestroySurface(mart_description_surface);

    /*
    SDL_Surface *vl_gothic_licence_surface = TTF_RenderText_Shaded_Wrapped(gui_state.font, vl_gothic_licence, 0, (SDL_Color){0xFF,0xFF,0xFF,0xFF}, (SDL_Color){0}, 0);
    vl_gothic_licence_texture = SDL_CreateTextureFromSurface(about_renderer, vl_gothic_licence_surface);
    SDL_DestroySurface(vl_gothic_licence_surface);
    */

    //ugly but TTF_RenderText_Shaded_Wrapped is bugged for me
    TTF_SetFontSize(gui_state.font, 50);
    SDL_Surface *vl_g_surface = Render_Text_lines(vl_gothic_licence, SDL_arraysize(vl_gothic_licence));
    vl_gothic_licence_texture = SDL_CreateTextureFromSurface(about_renderer, vl_g_surface);
    if (!vl_gothic_licence_texture)
    {
        SDL_Log("Error: couldnt create texture for vl-gothic-licence");
        About_Quit(NULL, SDL_APP_FAILURE);
    }
    SDL_DestroySurface(vl_g_surface);

    TTF_SetFontSize(gui_state.font, gui_state.font_point_size);
    return SDL_APP_CONTINUE;
}

void About_Compute_Scroll(float add_scroll)
{
    float about_scroll_y_max = about_content_size-about_h;
    if (about_scroll_y_max < 0)
        about_scroll_y_max = 0;
    about_scroll_y = SDL_clamp(about_scroll_y + (add_scroll * 10), -about_scroll_y_max, 0);
}

SDL_AppResult About_Event(void *appstate, SDL_Event *event)
{
    switch (event->type)
    {
    case SDL_EVENT_QUIT:
        return SDL_APP_SUCCESS;
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        About_Quit(appstate, SDL_APP_SUCCESS);
        break;
    case SDL_EVENT_KEY_DOWN:
        if (event->key.scancode == SDL_SCANCODE_ESCAPE)
            About_Quit(appstate, SDL_APP_SUCCESS);
    case SDL_EVENT_MOUSE_WHEEL:
        if (event->wheel.y)
            About_Compute_Scroll(event->wheel.y);
        break;
    }
    return SDL_APP_CONTINUE;
}
SDL_AppResult About_Iterate(void *appstate)
{
    About_Compute_Scroll(0);
    float offset_y = 0;
    SDL_GetWindowSize(about_window, &about_w, &about_h);
    SDL_SetRenderDrawColor(about_renderer, 0x18, 0x18, 0x18, 0xFF);
    SDL_RenderClear(about_renderer);


    {
        float scale = 150.0f / (float)logo_texture->h;
        SDL_FRect dst = (SDL_FRect){
            0,
            about_scroll_y + offset_y,
            logo_texture->w * scale,
            logo_texture->h * scale,
        };
        dst.x = about_w / 2 - dst.w / 2;
        SDL_RenderTexture(about_renderer, logo_texture, NULL, &dst);
        offset_y += dst.h;
    }
    if (mart_description_texture)
    {
        //float scale = (float)w / (float)mart_description_texture->w;
        SDL_FRect dst = (SDL_FRect){
            0,
            about_scroll_y + offset_y,
            mart_description_texture->w,
            mart_description_texture->h,
        };
        SDL_RenderTexture(about_renderer, mart_description_texture, NULL, &dst);
        offset_y += dst.h;
    }
    static bool vl_gothic_licence_dropdown = false;
    {
        SDL_FRect dst = (SDL_FRect){0,about_scroll_y + offset_y, about_w, 50};
        Gui_Toggle(about_renderer, dst, "VL-gothic licence/copyright", &vl_gothic_licence_dropdown);
        offset_y += dst.h;
    }
    if (vl_gothic_licence_dropdown)
    {
        float scale = (float)about_w / (float)vl_gothic_licence_texture->w;
        SDL_FRect dst = (SDL_FRect){
            0,
            about_scroll_y + offset_y,
            vl_gothic_licence_texture->w * scale,
            vl_gothic_licence_texture->h * scale,
        };
        SDL_RenderTexture(about_renderer, vl_gothic_licence_texture, NULL, &dst);
        offset_y += dst.h;
    }
    about_content_size = offset_y;

    SDL_RenderPresent(about_renderer);
    return SDL_APP_CONTINUE;
}
void About_Quit(void *appstate, SDL_AppResult result)
{
    if (logo_texture)
    {
        SDL_DestroyTexture(logo_texture);
        logo_texture = NULL;
    }
    if (mart_description_texture)
    {
        SDL_DestroyTexture(mart_description_texture);
        mart_description_texture = NULL;
    }
    if (vl_gothic_licence_texture)
    {
        SDL_DestroyTexture(vl_gothic_licence_texture);
        vl_gothic_licence_texture = NULL;
    }

    if (about_window)
    {
        SDL_DestroyWindow(about_window);
        about_window = NULL;
    }
    if (about_renderer)
    {
        SDL_DestroyRenderer(about_renderer);
        about_renderer = NULL;
    }
}
