#include "state.h"

SDL_Surface *logo_surface = NULL;
char *config_path = NULL;
char *font_path = NULL;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
int window_width = 800;
int window_height = 800;

SDL_Window *about_window = NULL;
SDL_Renderer *about_renderer = NULL;

struct mpd_connection *mpd = NULL;
char *active_album  = NULL;
char *active_artist = NULL;
char *active_date   = NULL;

bool is_bg_alt = false;
bool show_gui = true;

SDL_MouseButtonFlags last_frame_mouse_flags;

SDL_Texture *cover_art_texture = NULL;

bool keep_aspect_ratio = true;
