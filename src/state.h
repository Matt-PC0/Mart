#include <SDL3/SDL.h>
#include <mpd/client.h>

#ifndef _STATE_H_
#define _STATE_H_

static SDL_Surface *logo_surface = NULL;
static char *config_path = NULL;
static char *font_path = NULL;

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static int window_width = 800;
static int window_height = 800;

static SDL_Window *about_window = NULL;
static SDL_Renderer *about_renderer = NULL;

static struct mpd_connection *mpd = NULL;
static char *active_album  = NULL;
static char *active_artist = NULL;
static char *active_date   = NULL;

static bool is_bg_alt = false;
static bool show_gui = true;

static SDL_MouseButtonFlags last_frame_mouse_flags;

static SDL_Texture *cover_art_texture = NULL;

static bool keep_aspect_ratio = true;

#endif //ifndef _STATE_H_
