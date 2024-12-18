#ifndef MART_CONFIG_H
#define MART_CONFIG_H

//#include <raylib.h>
#include "toml.h"
#include "imgui.h"

typedef struct
{
    char *mpd_host;
    unsigned    mpd_port;
    unsigned    mpd_timeout_ms;

    SDL_Color background_color;
    SDL_Color background_color_alt;

    char *placeholder_image;

    bool use_volume_num_keys;
    unsigned volume_step;

    bool keep_aspect_ratio;
    bool show_gui;
    Uint64 fps_target;

    char *repeat_label;
    char *random_label;
    char *single_label;
    char *consume_label;
    char *crossfade_label;
    char *previous_label;
    char *play_label;
    char *pause_label;
    char *next_label;
} Config;
static const Config mart_config_default = 
{
.mpd_host             = NULL,
.mpd_port             = 0,
.mpd_timeout_ms       = 0,
.background_color     = {0x00,0x00,0x00,0xFF},
.background_color_alt = {0xFF,0xFF,0xFF,0xFF},
.placeholder_image    = NULL,
.use_volume_num_keys  = true,
.volume_step          = 10,
.keep_aspect_ratio    = true,
.show_gui             = true,
.fps_target           = 60,

.repeat_label         = "r",
.random_label         = "z",
.single_label         = "s",
.consume_label        = "c",
.crossfade_label      = "x",
.previous_label       = "<",
.play_label           = "+",
.pause_label          = "-",
.next_label           = ">",
};
extern Config mart_config;

typedef struct
{
    SDL_Keycode key;
    SDL_Keymod   mod;
} Keybind;

typedef struct
{
    Keybind play;
    Keybind next;
    Keybind previous;
    Keybind repeat;
    Keybind random;
    Keybind single;
    Keybind consume;
    Keybind crossfade;
    Keybind swap_color;
    Keybind toggle_gui;
    Keybind toggle_aspect_ratio;
    Keybind quit;
    Keybind reload_config;
    Keybind about;
    Keybind volume_up;
    Keybind volume_down;
} Key_Binds;
extern Key_Binds key_binds;


enum Str_To_Col_Err
{
    S_T_COL_ERR_NONE     = 0,
    S_T_COL_ERR_NULL_ARG = 1,
    S_T_COL_ERR_FORMAT1  = 2,
    S_T_COL_ERR_FORMAT2  = 3,
    S_T_COL_ERR_FORMAT3  = 4,
};
typedef enum Str_To_Col_Err Str_To_Col_Err;
typedef struct Toml_Colour Toml_Colour;
struct Toml_Colour
{
    bool ok;
    SDL_Color col;
};


SDL_Color Str_To_Col(const char *str, Str_To_Col_Err *err);
bool Toml_Array_Col(toml_array_t* array, SDL_Color *col);
Toml_Colour Toml_Col_In(toml_table_t *table, const char *key);
void Load_Theme(toml_table_t* toml_conf, const char *name, Gui_Colour *theme);
void Load_Config_Defaults(void);
void Load_Config(const char* file);

#endif //ifndef MART_CONFIG_H
