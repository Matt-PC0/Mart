#include <SDL3/SDL.h>
#include <mpd/client.h>

#ifndef _STATE_H_
#define _STATE_H_

extern SDL_Surface *logo_surface;
extern char *config_path;
extern char *font_path;

extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern int window_width;
extern int window_height;

extern SDL_Window *about_window;
extern SDL_Renderer *about_renderer;

extern struct mpd_connection *mpd;
extern char *active_album ;
extern char *active_artist;
extern char *active_date  ;

extern bool is_bg_alt;
extern bool show_gui;

extern SDL_MouseButtonFlags last_frame_mouse_flags;

extern SDL_Texture *cover_art_texture;

extern bool keep_aspect_ratio;

#endif //ifndef _STATE_H_
