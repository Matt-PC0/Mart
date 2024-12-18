
//#include <raymath.h>
#include <stdlib.h>
//#include <string.h>
#include "toml.h"

#include "config.h"
#include "toml.h"
#include "imgui.h"
#include "state.h"

#include "VL-Gothic.h"

Config mart_config = mart_config_default;
const Key_Binds key_binds_default = {
    .play                = { .key = SDLK_P     , .mod = 0 },
    .next                = { .key = SDLK_PERIOD, .mod = SDL_KMOD_SHIFT },
    .previous            = { .key = SDLK_COMMA , .mod = SDL_KMOD_SHIFT },
    .repeat              = { .key = SDLK_R     , .mod = 0 },
    .random              = { .key = SDLK_K     , .mod = 0 },
    .single              = { .key = SDLK_Y     , .mod = 0 },
    .consume             = { .key = SDLK_R     , .mod = SDL_KMOD_SHIFT },
    .crossfade           = { .key = SDLK_X     , .mod = 0 },
    .swap_color          = { .key = SDLK_SPACE , .mod = 0 },
    .toggle_gui          = { .key = SDLK_H     , .mod = 0 },
    .toggle_aspect_ratio = { .key = SDLK_RETURN, .mod = 0 },
    .quit                = { .key = SDLK_ESCAPE, .mod = 0 },
    .reload_config       = { .key = SDLK_U     , .mod = 0 },
    .about               = { .key = SDLK_I     , .mod = 0 },
    .volume_up           = { .key = SDLK_EQUALS, .mod = 0 },
    .volume_down         = { .key = SDLK_MINUS , .mod = 0 },
};
Key_Binds key_binds = key_binds_default;

unsigned char HexChar_To_Val(char ch, bool *valid)
{
    if (valid) *valid = true;

    if (ch >= '0' && ch <= '9')
        return ch - '0';
    else if (ch >= 'a' && ch <= 'f')
        return (ch - 'a') + 10;
    else if (ch >= 'A' && ch <= 'F')
        return (ch - 'A') + 10;

    if (valid) *valid = false;
    return 0;
}

SDL_Color Str_To_Col(const char *str, Str_To_Col_Err *err)
{
    SDL_Color col = (SDL_Color){0x00, 0x00, 0x00, 0xFF};
    if (str == NULL || err == NULL) return col;
    *err = S_T_COL_ERR_NONE;
    size_t str_len = strlen(str);
    if ( !(str_len == 7 || str_len == 9) || (str_len > 1 && str[0] != '#'))
    {
        *err = S_T_COL_ERR_FORMAT1;
        return (SDL_Color){0x00, 0x00, 0x00, 0x00};
    }

    // i+1 to skip '#'
    for (Uint32 i = 1; i < str_len; i += 2)
    {
        bool valid;
        unsigned char a = HexChar_To_Val(str[i], &valid);
        if (!valid) {
            *err = S_T_COL_ERR_FORMAT2;
            return (SDL_Color){0x00,0x00,0x00,0x00};
        }
        unsigned char b = HexChar_To_Val(str[i + 1], &valid);
        if (!valid) {
            *err = S_T_COL_ERR_FORMAT3;
            return (SDL_Color){0x00,0x00,0x00,0x00};
        }

        ((unsigned char*)&col)[(i-1)/2] = a*16+b;
    }

    return col;
}
bool Toml_Array_Col(toml_array_t* array, SDL_Color *col)
{
    if (col == NULL) return false;
    if (array == NULL) return false;
    SDL_Color tmp_col = (SDL_Color){0,0,0,0xff};

    int i = 0;
    for (;; i += 1)
    {
        toml_datum_t d = toml_int_at(array, i);
        if (!d.ok) break;
        if (d.u.i > 0xff || d.u.i < 0) return false;
        ((unsigned char*)&tmp_col)[i] = (unsigned char)d.u.i;
    }
    if (!(i == 3 || i == 4)) return false;
    col->r = tmp_col.r;
    col->g = tmp_col.g;
    col->b = tmp_col.b;
    col->a = tmp_col.a;
    return true;
}
Toml_Colour Toml_Col_In(toml_table_t *table, const char *key)
{
    Toml_Colour datum =
        {.col = (SDL_Color){0x00,0x00,0x00,0x00}, .ok = false};
    Str_To_Col_Err err = 0;
    toml_datum_t str = toml_string_in(table, key);
    datum.ok = str.ok;
    if (str.ok)
    {
        datum.col = Str_To_Col(str.u.s, &err);
        free(str.u.s);
        if (err)
        {
            printf("ERROR parseing colour string\n");
        }
        datum.ok = !err;
    }
    else
    {
        toml_array_t *col = toml_array_in(table, key);
        datum.ok = (bool)col;
        if (!Toml_Array_Col(col, &datum.col) && col != NULL)
        {
            datum.ok = false;
            printf("ERROR parseing colour array\n");
        }
    }
    return datum;
}
void Load_Theme(toml_table_t* toml_conf, const char *name, Gui_Colour *theme)
{
    toml_table_t *theme_conf = toml_table_in(toml_conf, name);
    if (theme_conf)
    {
        Toml_Colour text_datum = Toml_Col_In(theme_conf, "text");
        if (text_datum.ok)
        {
            theme->text = text_datum.col;
        }
        Toml_Colour bg = Toml_Col_In(theme_conf, "background");
        if (bg.ok)
        {
            theme->background = bg.col;
        }
        Toml_Colour border = Toml_Col_In(theme_conf, "border");
        if (border.ok)
        {
            theme->border = border.col;
        }
    }
}

Keybind Get_Keybind_From_String( const char *string )
{
    Keybind bind = {0};
    if (string == NULL)
        return bind;

    bind.key = SDL_GetKeyFromName(string);
    if (bind.key != SDLK_UNKNOWN)
        return bind;

    size_t string_len = SDL_strlen(string);
    char *string_copy = SDL_malloc(string_len + 1);
    SDL_strlcpy(string_copy, string, string_len + 1);

    static const int max_parts = 4;
    char *parts[max_parts];
    SDL_zero(parts);
    int part_n = 1;
    parts[0] = string_copy;
    for (Uint32 i = 0; i < string_len; i += 1)
    {
        if (string_copy[i] == '+' && i+1 < string_len)
        {
            if (part_n >= max_parts)
            {
                SDL_Log("ERROR: bind contains too many parts\n");
                SDL_free(string_copy);
                return bind;
            }
            string_copy[i] = '\0';
            parts[part_n] = string_copy + i + 1;
            part_n += 1;
        }
    }
    for (int i = 0; i < part_n-1; i += 1)
    {
        char *p = parts[i];
        size_t p_len = SDL_strnlen(p, string_len);
        if (p_len == 1)
        {
            switch (*p)
            {
            case 'S':
                bind.mod = bind.mod | SDL_KMOD_SHIFT;
                continue;
            case 'C':
                bind.mod = bind.mod | SDL_KMOD_CTRL;
                continue;
            case 'A':
                bind.mod = bind.mod | SDL_KMOD_ALT;
                continue;
            default:
                break;
            }
        }
        else
        {
            if (!SDL_strcmp(p, "SHIFT"))
            {
                bind.mod = bind.mod | SDL_KMOD_SHIFT;
                continue;
            }
            else if (!SDL_strcmp(p, "CTRL"))
            {
                bind.mod = bind.mod | SDL_KMOD_SHIFT;
                continue;
            }
            else if (!SDL_strcmp(p, "ALT"))
            {
                bind.mod = bind.mod | SDL_KMOD_ALT;
                continue;
            }
        }
        //skiped otherwise
        SDL_Log("ERROR: unkown mod '%s' in keybind", p);
        SDL_free(string_copy);
        return bind;
    }
    bind.key = SDL_GetKeyFromName(parts[part_n-1]);
    if (bind.key == SDLK_UNKNOWN)
        SDL_Log("ERROR: unknown keyname \"%s\" in bind \"%s\"\n", parts[part_n-1], string);

    SDL_free(string_copy);
    return bind;
}

void Load_Config_Defaults(void)
{
    key_binds = key_binds_default;

    if (mart_config.placeholder_image)
        free(mart_config.placeholder_image);

    if (mart_config.repeat_label && mart_config.repeat_label != mart_config_default.repeat_label)
        free(mart_config.repeat_label);
    if (mart_config.random_label && mart_config.random_label != mart_config_default.random_label)
        free(mart_config.random_label);
    if (mart_config.single_label && mart_config.single_label != mart_config_default.single_label)
        free(mart_config.single_label);
    if (mart_config.consume_label && mart_config.consume_label != mart_config_default.consume_label)
        free(mart_config.consume_label);
    if (mart_config.crossfade_label && mart_config.crossfade_label != mart_config_default.crossfade_label)
        free(mart_config.crossfade_label);
    if (mart_config.previous_label && mart_config.previous_label != mart_config_default.previous_label)
        free(mart_config.previous_label);
    if (mart_config.play_label && mart_config.play_label != mart_config_default.play_label)
        free(mart_config.play_label);
    if (mart_config.pause_label && mart_config.pause_label != mart_config_default.pause_label)
        free(mart_config.pause_label);
    if (mart_config.next_label && mart_config.next_label != mart_config_default.next_label)
        free(mart_config.next_label);

    mart_config = mart_config_default;

    if (gui_state.font) TTF_CloseFont(gui_state.font);
    gui_state = gui_state_default;

    if (!gui_state.font_point_size)
        gui_state.font_point_size = 50;
    if (font_path)
        gui_state.font = TTF_OpenFont(font_path,gui_state.font_point_size);
    else
    {
        SDL_IOStream *fallback_font_io = SDL_IOFromMem(fallback_font, fallback_font_size);
        gui_state.font = TTF_OpenFontIO(fallback_font_io, true, gui_state.font_point_size);
    }
}
void Load_Config(const char* file)
{

    FILE *fp = fopen(file, "r");
    if (!fp)
    {
        SDL_Log("WARNING: cant open config file: \"%s\"", file);
        return;
    }

    char errbuf[200];
    toml_table_t *toml_conf = toml_parse_file(fp, errbuf, sizeof(errbuf));
    fclose(fp);

    if (!toml_conf)
    {
        SDL_Log("FATAL: cannot parse config file: \"%s\" - %s", file, errbuf);
        exit(2);
    }

    toml_table_t *mpd_conf = toml_table_in(toml_conf, "mpd");
    if (mpd_conf)
    {
        toml_datum_t host = toml_string_in(mpd_conf, "host");
        if (host.ok) mart_config.mpd_host = host.u.s;
        toml_datum_t port = toml_int_in(mpd_conf, "port");
        if (port.ok) mart_config.mpd_port = port.u.i;
        toml_datum_t timeout_ms = toml_int_in(mpd_conf, "timeout_ms");
        if (timeout_ms.ok) mart_config.mpd_timeout_ms = host.u.i;
    }


    Load_Theme(toml_conf, "inactive_theme", &gui_state.inactive_col);
    Load_Theme(toml_conf, "active_theme", &gui_state.active_col);
    Load_Theme(toml_conf, "hover_theme", &gui_state.hover_col);

    toml_table_t *gui_conf = toml_table_in(toml_conf, "gui");
    if (gui_conf)
    {

        toml_datum_t font_point_size_datum = toml_double_in(gui_conf, "font_point_size");
        if (font_point_size_datum.ok)
            gui_state.font_point_size = font_point_size_datum.u.d;

        toml_datum_t font = toml_string_in(gui_conf, "font");
        if (font.ok) 
        {
            if (gui_state.font) TTF_CloseFont(gui_state.font);
            gui_state.font = TTF_OpenFont(font.u.s,gui_state.font_point_size);
            free(font.u.s);
        }
        else if (font_point_size_datum.ok)
        {
            if (gui_state.font) TTF_CloseFont(gui_state.font);
            if (font_path)
                gui_state.font = TTF_OpenFont(font_path,gui_state.font_point_size);
            else
            {
                SDL_IOStream *fallback_font_io = SDL_IOFromMem(fallback_font, fallback_font_size);
                gui_state.font = TTF_OpenFontIO(fallback_font_io, true, gui_state.font_point_size);
            }
        }
        toml_datum_t font_size = toml_int_in(gui_conf, "font_size");
        if (font_size.ok)
        {
            gui_state.font_size   =font_size.u.i;
            gui_state.conf_font_size =font_size.u.i;
        }
        toml_datum_t title_font_size = toml_int_in(gui_conf, "title_font_size");
        if (title_font_size.ok)
        {
            gui_state.title_font_size      =title_font_size.u.i;
            gui_state.conf_title_font_size =title_font_size.u.i;
        }
        toml_datum_t slider_text_margin = toml_int_in(gui_conf, "slider_text_margin");
        if (slider_text_margin.ok) gui_state.slider_text_margin = slider_text_margin.u.i;
        toml_datum_t border_size = toml_double_in(gui_conf, "border_size");
        if (border_size.ok) gui_state.border_size  = (float)border_size.u.d;

        toml_datum_t show_gui_datum = toml_bool_in(gui_conf, "show_gui");
        if (show_gui_datum.ok) mart_config.show_gui = show_gui_datum.u.b;

        toml_datum_t label = toml_string_in(gui_conf, "repeat_label");
        if (label.ok)
        {
            if (mart_config.repeat_label && mart_config.repeat_label != mart_config_default.repeat_label)
                free(mart_config.repeat_label);
            mart_config.repeat_label = label.u.s;
        }
        label = toml_string_in(gui_conf, "random_label");
        if (label.ok)
        {
            if (mart_config.random_label && mart_config.random_label != mart_config_default.random_label)
                free(mart_config.random_label);
            mart_config.random_label = label.u.s;
        }
        label = toml_string_in(gui_conf, "single_label");
        if (label.ok)
        {
            if (mart_config.single_label && mart_config.single_label != mart_config_default.single_label)
                free(mart_config.single_label);
            mart_config.single_label = label.u.s;
        }
        label = toml_string_in(gui_conf, "consume_label");
        if (label.ok)
        {
            if (mart_config.consume_label && mart_config.consume_label != mart_config_default.consume_label)
                free(mart_config.consume_label);
            mart_config.consume_label = label.u.s;
        }
        label = toml_string_in(gui_conf, "crossfade_label");
        if (label.ok)
        {
            if (mart_config.crossfade_label && mart_config.crossfade_label != mart_config_default.crossfade_label)
                free(mart_config.crossfade_label);
            mart_config.crossfade_label = label.u.s;
        }
        label = toml_string_in(gui_conf, "previous_label");
        if (label.ok)
        {
            if (mart_config.previous_label && mart_config.previous_label != mart_config_default.previous_label)
                free(mart_config.previous_label);
            mart_config.previous_label = label.u.s;
        }
        label = toml_string_in(gui_conf, "play_label");
        if (label.ok)
        {
            if (mart_config.play_label && mart_config.play_label != mart_config_default.play_label)
                free(mart_config.play_label);
            mart_config.play_label = label.u.s;
        }
        label = toml_string_in(gui_conf, "pause_label");
        if (label.ok)
        {
            if (mart_config.pause_label && mart_config.pause_label != mart_config_default.pause_label)
                free(mart_config.pause_label);
            mart_config.pause_label = label.u.s;
        }
        label = toml_string_in(gui_conf, "next_label");
        if (label.ok)
        {
            if (mart_config.next_label && mart_config.next_label != mart_config_default.next_label)
                free(mart_config.next_label);
            mart_config.next_label = label.u.s;
        }
    }
    toml_table_t *general_conf = toml_table_in(toml_conf, "general");
    if (general_conf)
    {
        toml_datum_t placeholder = toml_string_in(general_conf, "placeholder_image");
        if (placeholder.ok)
        {
            if (mart_config.placeholder_image)
            {
                free(mart_config.placeholder_image);
                mart_config.placeholder_image = NULL;
            }

            SDL_IOStream *placeholder_io = SDL_IOFromFile(placeholder.u.s, "r");
            if (placeholder_io)
            {
                SDL_CloseIO(placeholder_io);
                mart_config.placeholder_image = placeholder.u.s;
            }
            else
                printf("placeholder_image file: %s does not exist\n", placeholder.u.s);
        }

        Toml_Colour col = Toml_Col_In(general_conf, "color");
        if (col.ok)
            mart_config.background_color = col.col;
        Toml_Colour alt_col = Toml_Col_In(general_conf, "alt_color");
        if (alt_col.ok)
            mart_config.background_color_alt = alt_col.col;
        
        toml_datum_t fps = toml_int_in(general_conf, "fps");
        if (fps.ok)
            mart_config.fps_target = fps.u.i;

        toml_datum_t keep_ar = toml_bool_in(general_conf, "keep_aspect_ratio");
        if (keep_ar.ok)
            mart_config.keep_aspect_ratio = keep_ar.u.b;
    }

    toml_table_t *keybinds_conf = toml_table_in(toml_conf, "keybinds");
    if (keybinds_conf)
    {
        toml_datum_t key_d;
        key_d = toml_string_in(keybinds_conf, "play");
        if (key_d.ok)
            key_binds.play = Get_Keybind_From_String(key_d.u.s);
        free(key_d.u.s);
        key_d = toml_string_in(keybinds_conf, "next");
        if (key_d.ok)
            key_binds.next = Get_Keybind_From_String(key_d.u.s);
        free(key_d.u.s);
        key_d = toml_string_in(keybinds_conf, "previous");
        if (key_d.ok)
            key_binds.previous = Get_Keybind_From_String(key_d.u.s);
        free(key_d.u.s);
        key_d = toml_string_in(keybinds_conf, "repeat");
        if (key_d.ok)
            key_binds.repeat = Get_Keybind_From_String(key_d.u.s);
        free(key_d.u.s);
        key_d = toml_string_in(keybinds_conf, "random");
        if (key_d.ok)
            key_binds.random = Get_Keybind_From_String(key_d.u.s);
        free(key_d.u.s);
        key_d = toml_string_in(keybinds_conf, "single");
        if (key_d.ok)
            key_binds.single = Get_Keybind_From_String(key_d.u.s);
        free(key_d.u.s);
        key_d = toml_string_in(keybinds_conf, "consume");
        if (key_d.ok)
            key_binds.consume = Get_Keybind_From_String(key_d.u.s);
        free(key_d.u.s);
        key_d = toml_string_in(keybinds_conf, "crossfade");
        if (key_d.ok)
            key_binds.crossfade = Get_Keybind_From_String(key_d.u.s);
        free(key_d.u.s);
        key_d = toml_string_in(keybinds_conf, "swap_color");
        if (key_d.ok)
            key_binds.swap_color = Get_Keybind_From_String(key_d.u.s);
        free(key_d.u.s);
        key_d = toml_string_in(keybinds_conf, "toggle_gui");
        if (key_d.ok)
            key_binds.toggle_gui = Get_Keybind_From_String(key_d.u.s);
        free(key_d.u.s);
        key_d = toml_string_in(keybinds_conf, "toggle_aspect_ratio");
        if (key_d.ok)
            key_binds.toggle_aspect_ratio = Get_Keybind_From_String(key_d.u.s);
        free(key_d.u.s);
        key_d = toml_string_in(keybinds_conf, "quit");
        if (key_d.ok)
            key_binds.quit = Get_Keybind_From_String(key_d.u.s);
        free(key_d.u.s);
        key_d = toml_string_in(keybinds_conf, "reload_config");
        if (key_d.ok)
            key_binds.reload_config = Get_Keybind_From_String(key_d.u.s);
        free(key_d.u.s);
        key_d = toml_string_in(keybinds_conf, "about");
        if (key_d.ok)
            key_binds.about = Get_Keybind_From_String(key_d.u.s);
        free(key_d.u.s);
        key_d = toml_string_in(keybinds_conf, "volume_up");
        if (key_d.ok)
            key_binds.volume_up = Get_Keybind_From_String(key_d.u.s);
        free(key_d.u.s);
        key_d = toml_string_in(keybinds_conf, "volume_down");
        if (key_d.ok)
            key_binds.volume_down = Get_Keybind_From_String(key_d.u.s);
        free(key_d.u.s);

        key_d = toml_int_in(keybinds_conf, "volume_step");
        if (key_d.ok && key_d.u.i > 0)
            mart_config.volume_step = (unsigned)SDL_clamp(key_d.u.i, 1, 100);

        key_d = toml_bool_in(keybinds_conf, "use_volume_num_keys");
        if (key_d.ok)
            mart_config.use_volume_num_keys = key_d.u.b;
    }


    toml_free(toml_conf);
}
