#include "state.h"
/* unity build */
#include "album-art.c"
#include "config.c"
#include "compat.c"
#include "imgui.c"
#include "gui_controlls.c"

#include "logo.h"
#include "about_main.c"

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <mpd/client.h>

bool Is_Key_Pressed(SDL_KeyboardEvent key_event, Keybind keybind)
{
    bool is_mod_valid = (keybind.mod == key_event.mod) || (keybind.mod & ( key_event.mod & (SDL_KMOD_SHIFT | SDL_KMOD_CTRL | SDL_KMOD_ALT)));
    bool is_key_valid = key_event.key == keybind.key;
    return (is_key_valid && is_mod_valid);
}

void Handle_Keys( SDL_KeyboardEvent key )
{
    if (Is_Key_Pressed(key, key_binds.about))
    {
        if (!about_window)
            About_Init();
    }
    else if (Is_Key_Pressed(key, key_binds.swap_color)) 
    {
        is_bg_alt = !is_bg_alt;
    }
    else if (Is_Key_Pressed(key, key_binds.reload_config))
    {
        Load_Config_Defaults();
        Load_Config(config_path);
    }
    else if (Is_Key_Pressed(key, key_binds.quit)) 
    {
        exit(0);
    }
    else if (Is_Key_Pressed(key, key_binds.play)) 
    {
        if (key.repeat) return;
        mpd_run_toggle_pause(mpd);
    }
    else if (Is_Key_Pressed(key, key_binds.single)) 
    {
        struct mpd_status *status = mpd_run_status(mpd);
        if (status == NULL) return;
        mpd_run_single(mpd, !mpd_status_get_single(status));
        mpd_status_free(status);
    }
    else if (Is_Key_Pressed(key, key_binds.random)) 
    {
        struct mpd_status *status = mpd_run_status(mpd);
        if (status == NULL) return;
        mpd_run_random(mpd, !mpd_status_get_random(status));
        mpd_status_free(status);
    }
    else if (Is_Key_Pressed(key, key_binds.repeat)) 
    {
        struct mpd_status *status = mpd_run_status(mpd);
        if (status == NULL) return;
        mpd_run_repeat(mpd, !mpd_status_get_repeat(status));
        mpd_status_free(status);
    }
    else if (Is_Key_Pressed(key, key_binds.consume)) 
    {
        struct mpd_status *status = mpd_run_status(mpd);
        if (status == NULL) return;
        mpd_run_consume(mpd, !mpd_status_get_consume(status));
        mpd_status_free(status);
    }
    else if (Is_Key_Pressed(key, key_binds.crossfade)) 
    {
        struct mpd_status *status = mpd_run_status(mpd);
        if (status == NULL) return;
        mpd_run_crossfade(mpd, (mpd_status_get_crossfade(status)) ? 0 : 5);
        mpd_status_free(status);
    }
    else if (Is_Key_Pressed(key, key_binds.next)) 
    {
        mpd_run_next(mpd);
    }
    else if (Is_Key_Pressed(key, key_binds.previous)) 
    {
        mpd_run_previous(mpd);
    }
    else if (Is_Key_Pressed(key, key_binds.volume_up)) 
    {
        int volume = mpd_run_get_volume(mpd);
        if (volume >= 0)
            mpd_run_set_volume(mpd, (unsigned)SDL_clamp(volume + mart_config.volume_step, 0, 100));
    }
    else if (Is_Key_Pressed(key, key_binds.volume_down)) 
    {
        int volume = mpd_run_get_volume(mpd);
        if (volume >= 0)
            mpd_run_set_volume(mpd, (unsigned)SDL_clamp( volume - ((int)mart_config.volume_step), 0, 100));
    }
    else if (mart_config.use_volume_num_keys && key.key >= SDLK_0 && key.key <= SDLK_9)
    {
        unsigned volume = (key.key - SDLK_0) * 10;
        if (!volume) volume = 100;
        mpd_run_set_volume(mpd, volume);
    }

    else if (Is_Key_Pressed(key, key_binds.toggle_gui)) 
    {
        show_gui = !show_gui;
    }
    else if (Is_Key_Pressed(key, key_binds.toggle_aspect_ratio)) 
    {
        keep_aspect_ratio = !keep_aspect_ratio;
        if (keep_aspect_ratio)
        {
            float cover_art_aspect_ratio = (float)cover_art_texture->w/(float)cover_art_texture->h;
            SDL_SetWindowAspectRatio(window, cover_art_aspect_ratio, cover_art_aspect_ratio);
        }
        else
            SDL_SetWindowAspectRatio(window, 0, 0);
    }
}

bool mpd_song_copy_tag(char **dest, struct mpd_song *song, enum mpd_tag_type type, unsigned int idx)
{
    if (dest == NULL || song == NULL)
        return false;

    const char *tag = NULL;

    tag = mpd_song_get_tag(song, type, 0);
    if (tag == NULL)
        return false;

    size_t tag_size = SDL_strlen(tag) + 1;
    *dest = SDL_malloc(tag_size);
    SDL_strlcpy(*dest, tag, tag_size);

    return true;
}

bool Album_Has_Changed( const char *current_album, const char *current_artist, const char *current_date )
{
    bool album_tag_has_changed = false;
    if (active_album == NULL || current_album == NULL)
        album_tag_has_changed = active_album != current_album;
    else
        album_tag_has_changed = SDL_strcmp(active_album, current_album);

    bool artist_tag_has_changed = false;
    if (active_artist == NULL || current_artist == NULL)
        artist_tag_has_changed = active_artist != current_artist;
    else
        artist_tag_has_changed = SDL_strcmp(active_artist, current_artist);

    bool date_tag_has_changed = false;
    if (active_date == NULL || current_date == NULL)
        date_tag_has_changed = active_date != current_date;
    else
        date_tag_has_changed = SDL_strcmp(active_date, current_date);
        
    return album_tag_has_changed || artist_tag_has_changed || date_tag_has_changed;
}

void Process_Args(int argc, char *argv[])
{
    for (int i = 0; i < argc; i += 1)
    {
        if ( !SDL_strcmp(argv[i], "-c") || !SDL_strcmp(argv[i], "-config"))
        {
            if (i+1 < argc)
            {
                config_path = argv[i+1];
                i += 1;
            }
            else
                SDL_Log("WARNING: no config path provided");
        }
        else if ( !SDL_strcmp(argv[i], "-f") || !SDL_strcmp(argv[i], "-font") )
        {
            if (i+1 < argc)
            {
                font_path = argv[i+1];
                i += 1;
            }
            else
                SDL_Log("WARNING: no font path provided");
        }
        else if ( !SDL_strcmp(argv[i], "-i") || !SDL_strcmp(argv[i], "-image") )
        {
            if (i+1 < argc)
            {
                size_t arg_len = SDL_strlen(argv[i+1]);
                mart_config.placeholder_image = SDL_malloc(arg_len + 1);
                SDL_strlcpy(mart_config.placeholder_image, argv[i+1], arg_len+1);

                i += 1;
            }
            else
                SDL_Log("WARNING: no font path provided");
        }
    }
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Couldn't initialize SDL!", SDL_GetError(), NULL);
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("MART", window_width, window_height, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Couldn't create window/renderer!", SDL_GetError(), NULL);
        return SDL_APP_FAILURE;
    }

    logo_surface = IMG_Load_IO(SDL_IOFromMem(logo, logo_size), true);
    SDL_SetWindowIcon(window, logo_surface);
    //SDL_SetRenderVSync(renderer, 2);

    if (!TTF_Init())
    {
        SDL_Log("Couldnt initialise SDL_TTF! %s\n", SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Couldnt initialise SDL_TTF!\n", SDL_GetError(), NULL);
        return SDL_APP_FAILURE;
    }

    Load_Config_Defaults();
    Process_Args(argc, argv);

    if (config_path == NULL)
    {
        if (SDL_PLATFORM_UNIX) //might not work on all unix that dont support XDG_CONFIG_HOME
        {
            char *home = getenv("HOME");
            char *path = SDL_malloc(1000);
            SDL_snprintf(path, 1000, "%s/.config/mart/mart.toml", home);
            SDL_IOStream *io = SDL_IOFromFile(path, "r");
            if (io != NULL)
                config_path = path;
            else
                SDL_free(path);
            if (io) SDL_CloseIO(io);

        }
        if (config_path == NULL) //fallback to basedirectory (os dependent) on linux its the base path of the executable
        {
            char *path = SDL_malloc(1000);
            SDL_snprintf(path, 1000, "%smart.toml", SDL_GetBasePath());
            SDL_IOStream *io = SDL_IOFromFile(path, "r");
            if (io != NULL)
                config_path = path;
            else
                SDL_free(path);
            if (io) SDL_CloseIO(io);
        }
        if (config_path == NULL) //defaults it is
        {
            SDL_Log("Error: cant find config file");
        }
    }

    Load_Config(config_path);
    show_gui = mart_config.show_gui;

    mpd = mpd_connection_new(mart_config.mpd_host, mart_config.mpd_port, mart_config.mpd_timeout_ms);
    if (mpd == NULL)
    {
        SDL_Log("Couldnt connected to mpd!\n");
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Couldnt connected to mpd!\n", "Couldnt connected to mpd!\n", NULL);
        return SDL_APP_FAILURE;
    }

    struct mpd_song *song = mpd_run_current_song(mpd);
    cover_art_texture = Get_Album_Texture(renderer, mpd, song); //will handle song == NULL
    if (song) {
        mpd_song_copy_tag(&active_album, song, MPD_TAG_ALBUM, 0);
        mpd_song_copy_tag(&active_artist, song, MPD_TAG_ARTIST, 0);
        mpd_song_copy_tag(&active_date, song, MPD_TAG_DATE, 0);

        mpd_song_free(song);
    }
        
    SDL_Log("Initial album:\n");
    SDL_Log("album \"%s\", artist \"%s\", date \"%s\"\n", active_album, active_artist, active_date);

    keep_aspect_ratio = mart_config.keep_aspect_ratio;
    if (keep_aspect_ratio && cover_art_texture)
    {
        float cover_art_aspect_ratio = (float)cover_art_texture->w/(float)cover_art_texture->h;
        SDL_SetWindowAspectRatio(window, cover_art_aspect_ratio, cover_art_aspect_ratio);
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    SDL_Window *event_window = SDL_GetWindowFromEvent(event);
    if (event_window && event_window == about_window)
        return About_Event(appstate, event);

    switch (event->type)
    {
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
    case SDL_EVENT_QUIT:
        return SDL_APP_SUCCESS;
    case SDL_EVENT_KEY_DOWN:
        Handle_Keys(event->key);
        break;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    if (about_window && SDL_GetWindowID(about_window))
    {
        SDL_AppResult res = About_Iterate(appstate);
        if (res != SDL_APP_CONTINUE)
            return res;
    }
    Uint64 start_tick = SDL_GetTicks();
    SDL_GetWindowSize(window, &window_width, &window_height);
    gui_state.font_size = gui_state.conf_font_size*((float)window_height/800);
    gui_state.title_font_size = gui_state.conf_title_font_size*((float)window_height/800);

    struct mpd_song *song = mpd_run_current_song(mpd);
    const char *current_album = NULL;
    const char *current_artist = NULL;
    const char *current_date = NULL;

    if (song)
    {
        current_album = mpd_song_get_tag(song, MPD_TAG_ALBUM, 0);
        current_artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);
        current_date = mpd_song_get_tag(song, MPD_TAG_DATE, 0);
    }

    if ( Album_Has_Changed(current_album, current_artist, current_date) )
    {
        SDL_Log("\nAlbum changed!\n");
        SDL_Log("  old info:\n    %p: \"%s\"\n    %p: \"%s\"\n    %p: \"%s\"\n",
                active_album, active_album,
                active_artist, active_artist,
                active_date, active_date);
        if (active_album)
        {
            SDL_free(active_album);
            active_album = NULL;
        }
        if (active_artist)
        {
            SDL_free(active_artist);
            active_artist = NULL;
        }

        if (current_album)
        {
            size_t size = SDL_strlen(current_album) + 1;
            active_album = SDL_malloc(size);
            SDL_strlcpy(active_album, current_album, size);
        }
        else active_album = NULL;

        if (current_artist)
        {
            size_t size = SDL_strlen(current_artist) + 1;
            active_artist = SDL_malloc(size);
            SDL_strlcpy(active_artist, current_artist, size);
        }
        else active_artist = NULL;

        if (current_date)
        {
            size_t size = SDL_strlen(current_date) + 1;
            active_date = SDL_malloc(size);
            SDL_strlcpy(active_date, current_date, size);
        }
        else active_date = NULL;

        SDL_Log("  new info:\n    %p: \"%s\"\n    %p: \"%s\"\n    %p: \"%s\"\n",
                active_album, active_album,
                active_artist, active_artist,
                active_date, active_date);

        SDL_DestroyTexture(cover_art_texture);
        cover_art_texture = Get_Album_Texture(renderer, mpd, song);

        if (keep_aspect_ratio && cover_art_texture)
        {
            float cover_art_aspect_ratio = (float)cover_art_texture->w/(float)cover_art_texture->h;
            SDL_SetWindowAspectRatio(window, cover_art_aspect_ratio, cover_art_aspect_ratio);
        }

    }
    if (song) mpd_song_free(song);

    //SDL_SetRenderDrawColorrenderer, 0x18, 0x18, 0x18, 0xff);
    if (!is_bg_alt)
        SDL_SetRenderDrawColor(renderer, mart_config.background_color.r, mart_config.background_color.g, mart_config.background_color.b, mart_config.background_color.a);
    else
        SDL_SetRenderDrawColor(renderer, mart_config.background_color_alt.r, mart_config.background_color_alt.g, mart_config.background_color_alt.b, mart_config.background_color_alt.a);
        
    SDL_RenderClear(renderer);


    if (cover_art_texture)
    {
        float scale = (window_width < window_height) ?
            (float)window_width / (float)cover_art_texture->w :
            (float)window_height / (float)cover_art_texture->h;

        SDL_FRect rect = { 0 };
        rect.w = cover_art_texture->w * scale;
        rect.h = cover_art_texture->h * scale;
        rect.x = window_width/2 - rect.w/2;
        rect.y = window_height/2 - rect.h/2;
        SDL_RenderTexture(renderer, cover_art_texture, NULL, &rect);
    }

    Gui_Volume_Controll(mpd, show_gui);
    Gui_Mode_Controlls(mpd, show_gui);
    Gui_Player_Controlls(mpd, show_gui);
    Gui_Title(mpd, show_gui);

    last_frame_mouse_flags = SDL_GetMouseState(NULL, NULL);

    const double delay = 1.0/((double)mart_config.fps_target)*1000.0 - (SDL_GetTicks() - start_tick);
    SDL_Delay( (delay > 0) ? delay : 0 ); //if frame time is slower then target then delay(0)

    SDL_RenderPresent(renderer);
    
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    mpd_connection_free(mpd);

    if (cover_art_texture)
        SDL_DestroyTexture(cover_art_texture);
    if (logo_surface)
        SDL_DestroySurface(logo_surface);

    if (window)
        SDL_DestroyWindow(window);
    if (renderer)
        SDL_DestroyRenderer(renderer);
}
