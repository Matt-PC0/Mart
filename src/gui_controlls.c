
#include "gui_controlls.h"
#include "imgui.h"
#include "state.h"
#include "compat.h"
#include "config.h"

void Gui_Player_Controlls(struct mpd_connection *mpd, bool show_gui)
{
    static float gui_lerp_amount = 0;
    static double gui_lerp_timestamp = 0;
    static bool was_gui_shown = 0;
    static float gui_controlls_y = -1;
    static bool was_sliding = false;
    static float progress_slider = 0;
    static float h_v = 0;

    float mouse_y;
    float mouse_x;
    SDL_GetMouseState(&mouse_x, &mouse_y);
    float button_w = ((float)window_width)/10;
    float button_h = ((float)window_height)/10;
    float button_gap_w = ((float)window_width)/100;
    float button_gap_h = ((float)window_height)/100;

    bool is_playing     = 0;
    bool toggle_playing = 0;
    struct mpd_status *status = mpd_run_status(mpd);
    if (status)
    {
        enum mpd_state state = mpd_status_get_state(status);
        if (!was_sliding) progress_slider = ((float)mpd_status_get_elapsed_ms(status))/1000;
        mpd_status_free(status);
        is_playing = (state == MPD_STATE_PLAY);
        toggle_playing = is_playing;
    }

    struct mpd_song * song = mpd_run_current_song(mpd);
    float duration = 0;
    if (song != NULL) duration = ((float)mpd_song_get_duration_ms(song)) / 1000;
    if (song != NULL) mpd_song_free(song);

    bool is_gui_shown = (show_gui && Is_Window_Focused( window ));
    if (is_gui_shown != was_gui_shown) gui_lerp_timestamp = GetTime();
    gui_lerp_amount = SDL_clamp(GetTime()-gui_lerp_timestamp, 0, 1);
    if (is_gui_shown) //slide in
    {
        if (gui_controlls_y < 0) gui_controlls_y = ((float)window_height);
        float l = SDL_clamp((mouse_y-((float)window_height)/2)/(((float)window_height)/2-button_h), 0, 1);
        gui_controlls_y = Lerp(((float)window_height), ((float)window_height)-button_h-button_gap_h, l);
    }
    else //slide out
    {
        gui_controlls_y = Lerp(gui_controlls_y, ((float)window_height), gui_lerp_amount);
    }
    was_gui_shown = is_gui_shown;

    if (Gui_Button(renderer, (SDL_FRect){button_gap_w, gui_controlls_y, button_w,button_h}, mart_config.previous_label))
    {
        mpd_run_previous(mpd);
    }

    Gui_Toggle(renderer, (SDL_FRect){button_gap_w*2+button_w, gui_controlls_y, button_w,button_h}, (toggle_playing) ? mart_config.play_label : mart_config.pause_label, &toggle_playing);
    {
        if (toggle_playing != is_playing)
        {
            mpd_run_toggle_pause(mpd);
        }
    }
    if (Gui_Button(renderer, (SDL_FRect){button_gap_w*3+button_w*2, gui_controlls_y, button_w,button_h}, mart_config.next_label))
    {
        mpd_run_next(mpd);
    }
    SDL_FRect slider_rect = (SDL_FRect){button_gap_w*4+button_w*3, gui_controlls_y, ((float)window_width) - (button_gap_w*4+button_w*3) - button_gap_w,button_h};

    char text_min[1000], text_max[1000], text_hover[1000];
    SDL_snprintf(text_min, 1000, "%02i:%02i", (int)(progress_slider/60),(int)progress_slider-(int)(progress_slider/60)*60);
    SDL_snprintf(text_hover, 1000, "%02i:%02i", (int)(h_v/60),(int)h_v-(int)(h_v/60)*60);
    SDL_snprintf(text_max, 1000, "%02i:%02i", (int)((duration-progress_slider)/60), (int)(duration-progress_slider) - (int)((duration-progress_slider)/60)*60);
    bool is_sliding = Gui_Slider(renderer, slider_rect, text_min, text_hover, text_max, SLIDER_HOVER_ABOVE_N, &progress_slider, &h_v, 0, duration, SLIDER_LTR);

    // if progress_slider == duration was_sliding is true thus at the end of every song it infinatly seeks to the end and not only never continues to the next in que but also causes mpd to leak cpu
    if ( ((is_sliding && !was_sliding) || (!is_sliding && was_sliding)) && progress_slider < duration)
    {
        was_sliding = false;
        mpd_run_seek_current(mpd, progress_slider, false);
    }
    was_sliding = is_sliding;
}

void Gui_Title(struct mpd_connection *mpd, bool show_gui)
{
    if (gui_state.font == NULL)
        return;

    static bool was_gui_shown       = 0;
    static double gui_lerp_timestamp = 0;
    static float gui_controlls_y    = 0;

    float mouse_y;
    SDL_GetMouseState(NULL, &mouse_y);
    double time_now = GetTime();

    float l;

    bool is_gui_shown = (show_gui && Is_Window_Focused( window ));
    if (is_gui_shown != was_gui_shown) gui_lerp_timestamp = time_now;

    if (is_gui_shown) //slide in
    {
        l = SDL_clamp(((((float)window_height)-mouse_y)-((float)window_height)/2)/(((float)window_height)/2-(gui_state.title_font_size)), 0,1);
        gui_controlls_y = Lerp(-gui_state.title_font_size, 0, l);
    }
    else //slide out
    {
        l = SDL_clamp(time_now-gui_lerp_timestamp, 0.0, 1.0);
        gui_controlls_y = Lerp(gui_controlls_y, -(gui_state.title_font_size), l);
    }
    was_gui_shown = is_gui_shown;

    Fill_Rect(renderer, (SDL_FRect){0,gui_controlls_y, ((float)window_width), gui_state.title_font_size}, (SDL_Color){0x00,0x00,0x00,0xFF*0.5});
    struct mpd_song* song = mpd_run_current_song(mpd);
    if (song != NULL)
    {
        const char* title = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
        const char* album = mpd_song_get_tag(song, MPD_TAG_ALBUM, 0);
        const char* artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);
        char text[1000];
        SDL_snprintf(text, 1000, "%s - %s - %s", title, album, artist);
        SDL_Surface *text_surface = TTF_RenderText_Shaded(gui_state.font, text, 0, (SDL_Color){ 0xFF,0xFF,0xFF,0xFF}, (SDL_Color){ 0 });
        SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
        float scale = gui_state.title_font_size / text_texture->h;
        SDL_DestroySurface(text_surface);
        SDL_FPoint text_size = {
            .x = text_texture->w * scale,
            .y = text_texture->h * scale,
        };
        float text_x = 0;
        if (text_size.x > ((float)window_width))
        {
            //ease in ease out Sine
            text_x = (text_size.x - ((float)window_width)) * -(-(SDL_cos(time_now*0.5)-1)/2);
        }
        SDL_RenderTexture(renderer, text_texture, NULL, &(SDL_FRect){ text_x, gui_controlls_y, text_texture->w * scale, text_texture->h * scale });
        SDL_DestroyTexture(text_texture);
    }
    else
        Draw_Text(gui_state.font, (SDL_FPoint){0, gui_controlls_y}, gui_state.title_font_size, "No Song Playing", (SDL_Color){ 0xFF,0xFF,0xFF,0xFF}, (SDL_Color){0});

    if (song) mpd_song_free(song);

}

// XXX: cant change or veiw volume when mpd que is empty, seems to be the same in ncmpcpp
void Gui_Volume_Controll(struct mpd_connection *mpd, bool show_gui)
{
    static bool  was_gui_shown      = 0;
    static double gui_lerp_timestamp = 0;
    static float gui_controlls_x    = 0;
    static float h_v                = 0;

    float mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);
    float button_w = ((float)window_width)/10;
    float button_h = ((float)window_height)/10;
    float button_gap_w = ((float)window_width)/100;
    float button_gap_h = ((float)window_height)/100;

    double time_now = GetTime();

    bool is_window_focused = Is_Window_Focused( window );
    bool is_gui_shown = (show_gui && is_window_focused);
    if (is_gui_shown != was_gui_shown) gui_lerp_timestamp = time_now;

    SDL_FRect slider_rec;
    slider_rec.x = ((float)window_width) - (button_gap_w+button_w);
    slider_rec.y = button_h + button_gap_h;
    slider_rec.w = button_w;
    slider_rec.h = ((float)window_height) - slider_rec.y - button_h - button_gap_h*2;

    float l;
    if (is_gui_shown) //slide in
    {
        l = SDL_clamp((mouse_x-((float)window_width)/2)/(((float)window_width)/2-(((float)window_width)-slider_rec.x)), 0, 1);
        gui_controlls_x = Lerp(((float)window_width), slider_rec.x, l);
    }
    else //slide out
    {
        l = SDL_clamp(time_now-gui_lerp_timestamp, 0.0, 1.0);
        gui_controlls_x = Lerp(gui_controlls_x, ((float)window_width), l);
    }
    was_gui_shown = is_gui_shown;


    slider_rec.x = gui_controlls_x;

    bool is_hovered = Is_Window_Focused(SDL_GetRenderWindow(renderer))
                   &&  mouse_x > slider_rec.x && mouse_x < slider_rec.x + slider_rec.w
                   &&  mouse_y > slider_rec.y && mouse_y < slider_rec.y + slider_rec.h;

    float volume = (float)mpd_run_get_volume(mpd);
    char text_min[1000], text_hover[1000];
    SDL_snprintf(text_min, 1000, "%i%%", (int)volume);
    SDL_snprintf(text_hover, 1000, "%i%%", (int)SDL_round(h_v));
    bool is_sliding = Gui_Slider(renderer, slider_rec, text_min, text_hover, NULL, SLIDER_HOVER_BELOW_P, &volume, &h_v, 0,100, SLIDER_BTT);
    if (is_sliding)
    {
        mpd_run_set_volume(mpd, (unsigned)SDL_clamp(SDL_round(volume), 0, 100));
    }
    else if (is_hovered && Is_Mouse_Released(SDL_BUTTON_RMASK))
    {
        mpd_run_set_volume(mpd, 100);
    }
}

void Gui_Mode_Controlls(struct mpd_connection *mpd, bool show_gui)
{
    static bool was_gui_shown       = 0;
    static float gui_lerp_timestamp = 0;
    static float gui_controlls_x    = 0;

    float mouse_x;
    SDL_GetMouseState(&mouse_x, NULL);
    float button_w = ((float)window_width)/10;
    float button_h = ((float)window_height)/10;
    float button_gap_w = ((float)window_width)/100;
    float button_gap_h = ((float)window_height)/100;

    bool is_gui_shown = (show_gui && Is_Window_Focused( window ));
    if (is_gui_shown != was_gui_shown) gui_lerp_timestamp = GetTime();

    SDL_FRect button_rec;
    button_rec.x      = button_gap_w;
    button_rec.y      = ((float)window_height)/2 - (button_h+(button_gap_h+button_h)*4)/2;
    button_rec.w  = button_w;
    button_rec.h = button_h;

    if (is_gui_shown) //slide in
    {
        float l = SDL_clamp(((((float)window_width)-mouse_x)-((float)window_width)/2)/(((float)window_width)/2-(button_rec.x+button_rec.w)), 0,1);
        gui_controlls_x = Lerp(-(button_rec.x+button_rec.w), button_rec.x, l);
    }
    else //slide out
    {
        float gui_lerp_amount = SDL_clamp(GetTime()-gui_lerp_timestamp, 0, 1);
        gui_controlls_x = Lerp(gui_controlls_x, -(button_rec.x + button_rec.w), gui_lerp_amount);
    }
    button_rec.x = gui_controlls_x;

    struct mpd_status *status = mpd_run_status(mpd);

    bool repeat = (status) ? mpd_status_get_repeat(status) : false;
    if (Gui_Toggle(renderer, button_rec, mart_config.repeat_label, &repeat))
    {
        mpd_run_repeat(mpd, repeat);
    }
    button_rec.y += button_rec.h + button_gap_h;
    bool random = (status) ? mpd_status_get_random(status) : false;
    if (Gui_Toggle(renderer, button_rec, mart_config.random_label, &random))
    {
        mpd_run_random(mpd, random);
    }
    button_rec.y += button_rec.h + button_gap_h;
    bool single = (status) ? mpd_status_get_single(status) : false;
    if (Gui_Toggle(renderer, button_rec, mart_config.single_label, &single))
    {
        mpd_run_single(mpd, single);
    }
    button_rec.y += button_rec.h + button_gap_h;
    bool consume = (status) ? mpd_status_get_consume(status) : false;
    if (Gui_Toggle(renderer, button_rec, mart_config.consume_label, &consume))
    {
        mpd_run_consume(mpd, consume);
    }
    button_rec.y += button_rec.h + button_gap_h;
    unsigned crossfade = (status) ? mpd_status_get_crossfade(status) : false;
    bool crossfade_toggle = crossfade != 0;
    if (Gui_Toggle(renderer, button_rec, mart_config.crossfade_label, &crossfade_toggle))
    {
        mpd_run_crossfade(mpd, (crossfade_toggle) ? 5 : 0);
    }

    if (status)
        mpd_status_free(status);
}
