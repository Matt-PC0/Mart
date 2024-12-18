
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "imgui.h"
#include "state.h"
#include "compat.h"

bool Gui_Button_State(SDL_Renderer *renderer, SDL_FRect rect, const char* text, Gui_State state)
{
    SDL_FPoint mouse_pos;
    SDL_MouseButtonFlags mouse_buttons = SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
    bool is_hovered = Is_Window_Focused(SDL_GetRenderWindow(renderer))
                   &&  mouse_pos.x > rect.x && mouse_pos.x < rect.x + rect.w
                   &&  mouse_pos.y > rect.y && mouse_pos.y < rect.y + rect.h;
    bool is_pressed = (is_hovered && Is_Mouse_Pressed(SDL_BUTTON_LMASK));

    Gui_Colour col = (is_pressed) ? state.active_col : (is_hovered) ? state.hover_col : state.inactive_col;

    SDL_FRect inner_rect = rect;
    inner_rect.x += state.border_size;
    inner_rect.y += state.border_size;
    inner_rect.w -= state.border_size*2;
    inner_rect.h -= state.border_size*2;
    Fill_Rect(renderer, inner_rect, col.background);
    Draw_Rect(renderer, rect, col.border, state.border_size);

    if (text != NULL && gui_state.font != NULL)
    {
        SDL_Surface *text_surface = TTF_RenderText_Shaded(state.font, text, 0, col.text, (SDL_Color){ 0 });
        SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
        float scale = state.font_size / text_texture->h;
        SDL_DestroySurface(text_surface);
        SDL_FPoint text_size = {
            .x = text_texture->w * scale,
            .y = text_texture->h * scale,
        };

        SDL_FPoint text_pos;
        text_pos.x = rect.x+rect.w/2 - text_size.x/2;
        text_pos.y = rect.y+rect.h/2 - text_size.y/2;
        SDL_RenderTexture(renderer, text_texture, NULL, &(SDL_FRect){ text_pos.x, text_pos.y, text_texture->w * scale, text_texture->h * scale });
        SDL_DestroyTexture(text_texture);
    }

    return is_pressed;
}

bool Gui_Button(SDL_Renderer *renderer, SDL_FRect rect, const char* text)
{
    return Gui_Button_State(renderer, rect, text, gui_state);
}

bool Gui_Toggle_State(SDL_Renderer *renderer, SDL_FRect rect, const char* text, bool *toggle, Gui_State state)
{
    bool fallback_toggle = false;
    //shouldnt be the case
    if (toggle == NULL) toggle = &fallback_toggle;

    SDL_FPoint mouse_pos;
    SDL_MouseButtonFlags mouse_buttons = SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
    bool is_hovered = Is_Window_Focused(SDL_GetRenderWindow(renderer))
                   &&  mouse_pos.x > rect.x && mouse_pos.x < rect.x + rect.w
                   &&  mouse_pos.y > rect.y && mouse_pos.y < rect.y + rect.h;
    bool is_pressed = (is_hovered && Is_Mouse_Pressed(SDL_BUTTON_LMASK));
    if (is_pressed) *toggle = !(*toggle);

    Gui_Colour col = (is_hovered) ? state.hover_col : (*toggle) ? state.active_col : state.inactive_col;
    if (is_hovered && Is_Mouse_Down(SDL_BUTTON_LMASK)) col = (!*toggle) ? state.active_col : state.inactive_col;

    SDL_FRect inner_rect = rect;
    inner_rect.x += state.border_size;
    inner_rect.y += state.border_size;
    inner_rect.w -= state.border_size*2;
    inner_rect.h -= state.border_size*2;
    Fill_Rect(renderer, inner_rect, col.background);
    Draw_Rect(renderer, rect, col.border, state.border_size);

    if (text != NULL && gui_state.font != NULL)
    {

        SDL_Surface *text_surface = TTF_RenderText_Shaded(state.font, text, 0, col.text, (SDL_Color){ 0 });
        SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
        float scale = state.font_size / text_texture->h;
        SDL_DestroySurface(text_surface);
        SDL_FPoint text_size = {
            .x = text_texture->w * scale,
            .y = text_texture->h * scale,
        };

        SDL_FRect text_pos;
        text_pos.x = rect.x+rect.w/2 - text_size.x/2;
        text_pos.y = rect.y+rect.h/2 - text_size.y/2;
        SDL_RenderTexture(renderer, text_texture, NULL, &(SDL_FRect){ text_pos.x, text_pos.y, text_texture->w * scale, text_texture->h * scale });
        SDL_DestroyTexture(text_texture);
    }

    return is_pressed;
}

bool Gui_Toggle(SDL_Renderer *renderer, SDL_FRect rect, const char* text, bool *toggle)
{
    return Gui_Toggle_State(renderer, rect, text, toggle, gui_state);
}

bool Gui_Slider_State(SDL_Renderer *renderer, SDL_FRect rect, const char* text_min, const char *text_hover, const char* text_max, Slider_Hover_Text_Pos hover_text_pos, float *value, float *hover_value, float min, float max, Slider_Orientation orientation , Gui_State state)
{
    float fallback_value = 0.0f;
    //shouldnt be the case
    if (value == NULL) value = &fallback_value;

    SDL_FPoint mouse_pos;
    SDL_MouseButtonFlags mouse_buttons = SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
    bool is_hovered = Is_Window_Focused(SDL_GetRenderWindow(renderer))
                   &&  mouse_pos.x > rect.x && mouse_pos.x < rect.x + rect.w
                   &&  mouse_pos.y > rect.y && mouse_pos.y < rect.y + rect.h;
    bool is_pressed = (is_hovered && Is_Mouse_Down(SDL_BUTTON_LMASK));

    Gui_Colour col = (is_hovered) ? state.hover_col : state.inactive_col;

    SDL_FRect inner_rect = rect;
    inner_rect.x += state.border_size;
    inner_rect.y += state.border_size;
    inner_rect.w -= state.border_size*2;
    inner_rect.h -= state.border_size*2;
    Fill_Rect(renderer, inner_rect, state.inactive_col.background);
    Draw_Rect(renderer, rect, col.border, state.border_size);

    SDL_FRect hover_bar = inner_rect;
    SDL_FRect progress_bar = inner_rect;

    float norm;
    switch (orientation)
    {
        case SLIDER_LTR:
            norm = (mouse_pos.x-inner_rect.x)/inner_rect.w;
            hover_bar.x = inner_rect.x + (inner_rect.w * norm) - state.border_size;
            hover_bar.w = state.border_size;

            progress_bar.w *= (*value-min)/max-min;
            break;
        case SLIDER_RTL:
            norm = 1 - (mouse_pos.x-inner_rect.x)/inner_rect.w;
            hover_bar.x = inner_rect.x + inner_rect.w - (inner_rect.w * norm) - state.border_size;
            hover_bar.w = state.border_size;

            progress_bar.x = inner_rect.x + inner_rect.w - inner_rect.w * (*value-min)/max-min;
            progress_bar.w -= progress_bar.x - inner_rect.x;
            break;
        case SLIDER_TTB:
            norm = (mouse_pos.y-inner_rect.y)/inner_rect.h;
            hover_bar.y = inner_rect.y + inner_rect.h * norm - state.border_size;
            hover_bar.h = state.border_size;

            progress_bar.h *= (*value-min)/max-min;
            break;
        case SLIDER_BTT:
            norm = 1 - (mouse_pos.y-inner_rect.y)/inner_rect.h;

            hover_bar.y = inner_rect.y + inner_rect.h - inner_rect.h * norm - state.border_size;
            hover_bar.h = state.border_size;

            progress_bar.y = inner_rect.y + inner_rect.h - inner_rect.h * (*value-min)/max-min;
            progress_bar.h -= progress_bar.y - inner_rect.y;
            break;
    }

    float delta = max-min;
    float h_v = min + delta*norm;
    if (hover_value) *hover_value = h_v;

    if (is_pressed)
    {
        //float delta = max-min;
        //*value = min + delta*norm;
        *value = h_v;
    }

    Fill_Rect(renderer, progress_bar, state.active_col.background);
    if (is_hovered) Fill_Rect(renderer, hover_bar, col.border);


    if (text_min != NULL && gui_state.font != NULL)
    {

        SDL_Surface *text_surface = TTF_RenderText_Shaded(state.font, text_min, 0, col.text, (SDL_Color){ 0 });
        SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
        float scale = state.font_size / text_texture->h;
        SDL_DestroySurface(text_surface);
        SDL_FPoint text_size = {
            .x = text_texture->w * scale,
            .y = text_texture->h * scale,
        };

        //Vector2 text_size = MeasureTextEx(state.font, text_min, state.font_size, state.font_spaceing);

        SDL_FRect text_pos;
        switch (orientation)
        {
            case SLIDER_LTR:
                text_pos.y = inner_rect.y+inner_rect.h/2 - text_size.y/2;
                text_pos.x = inner_rect.x + state.slider_text_margin;
                break;
            case SLIDER_RTL:
                text_pos.y = inner_rect.y+inner_rect.h/2 - text_size.y/2;
                text_pos.x = inner_rect.x+inner_rect.w-text_size.x - state.slider_text_margin;
                break;
            case SLIDER_TTB:
                text_pos.y = inner_rect.y + state.slider_text_margin;
                text_pos.x = inner_rect.x+inner_rect.w/2 - text_size.x/2;
                break;
            case SLIDER_BTT:
                text_pos.y = inner_rect.y+inner_rect.h-text_size.y - state.slider_text_margin;
                text_pos.x = inner_rect.x+inner_rect.w/2 - text_size.x/2;
                break;
        }
        SDL_RenderTexture(renderer, text_texture, NULL, &(SDL_FRect){ text_pos.x, text_pos.y, text_texture->w * scale, text_texture->h * scale });
        SDL_DestroyTexture(text_texture);
    }
    if (text_max != NULL && gui_state.font != NULL)
    {
        SDL_Surface *text_surface = TTF_RenderText_Shaded(state.font, text_max, 0, col.text, (SDL_Color){ 0 });
        SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
        float scale = state.font_size / text_texture->h;
        SDL_DestroySurface(text_surface);
        SDL_FPoint text_size = {
            .x = text_texture->w * scale,
            .y = text_texture->h * scale,
        };

        SDL_FPoint text_pos;
        switch (orientation)
        {
            case SLIDER_LTR:
                text_pos.y = inner_rect.y+inner_rect.h/2 - text_size.y/2;
                text_pos.x = inner_rect.x+inner_rect.w-text_size.x - state.slider_text_margin;
                break;
            case SLIDER_RTL:
                text_pos.y = inner_rect.y+inner_rect.h/2 - text_size.y/2;
                text_pos.x = inner_rect.x + state.slider_text_margin;
                break;
            case SLIDER_TTB:
                text_pos.y = inner_rect.y+inner_rect.h-text_size.y - state.slider_text_margin;
                text_pos.x = inner_rect.x+inner_rect.w/2 - text_size.x/2;
                break;
            case SLIDER_BTT:
                text_pos.y = inner_rect.y + state.slider_text_margin;
                text_pos.x = inner_rect.x+inner_rect.w/2 - text_size.x/2;
                break;
        }
        SDL_RenderTexture(renderer, text_texture, NULL, &(SDL_FRect){ text_pos.x, text_pos.y, text_texture->w * scale, text_texture->h * scale });
        SDL_DestroyTexture(text_texture);
    }

    if (text_hover != NULL && gui_state.font != NULL && is_hovered)
    {
        SDL_Surface *text_surface = TTF_RenderText_Shaded(state.font, text_hover, 0, col.text, (SDL_Color){ 0 });
        SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
        float scale = state.font_size / text_texture->h;
        SDL_DestroySurface(text_surface);
        SDL_FPoint text_size = {
            .x = text_texture->w * scale,
            .y = text_texture->h * scale,
        };
        SDL_FPoint text_pos;

        float *text_pos_const;
        float *text_pos_var;
        SDL_FRect inner_rect_n;
        SDL_FPoint text_n;
        SDL_FPoint mouse_n;
        int direction = 0;
        switch (orientation)
        {
            case SLIDER_LTR:
                direction = 1;
            case SLIDER_RTL:
                if (!direction)
                    direction = -1;
                text_pos_const = &text_pos.x;
                text_pos_var = &text_pos.y;
                inner_rect_n = inner_rect;
                text_n = text_size;
                mouse_n = mouse_pos;
                break;
            case SLIDER_TTB:
                direction = 1;
            case SLIDER_BTT:
                if (!direction)
                    direction = -1;
                text_pos_const = &text_pos.y;
                text_pos_var = &text_pos.x;
                inner_rect_n = (SDL_FRect){
                    inner_rect.y,
                    inner_rect.x,
                    inner_rect.h,
                    inner_rect.w,
                };
                text_n = (SDL_FPoint){text_size.y, text_size.x};
                mouse_n = (SDL_FPoint){mouse_pos.y, mouse_pos.x};
                break;
        }

        *text_pos_const = mouse_n.x - text_n.x/2;
        if (hover_text_pos >= _SLIDER_HOVER_P_START_ && hover_text_pos <= _SLIDER_HOVER_P_END_)
            *text_pos_const += text_n.x/2*direction;
        else if (hover_text_pos >= _SLIDER_HOVER_N_START_ && hover_text_pos <= _SLIDER_HOVER_N_END_)
            *text_pos_const -= text_n.x/2*direction;

        switch (hover_text_pos)
        {
        case SLIDER_HOVER_MID_P:
        case SLIDER_HOVER_MID_N:
        case SLIDER_HOVER_MID:
            *text_pos_var = inner_rect_n.y + inner_rect_n.h/2 - text_n.y /2;
            break;
        case SLIDER_HOVER_FOLLOW_P:
        case SLIDER_HOVER_FOLLOW_N:
        case SLIDER_HOVER_FOLLOW:
            *text_pos_var = mouse_n.y - text_n.y /2;
            break;
        case SLIDER_HOVER_ABOVE_P:
        case SLIDER_HOVER_ABOVE_N:
        case SLIDER_HOVER_ABOVE:
            *text_pos_var = inner_rect_n.y + gui_state.slider_text_margin;
            break;
        case SLIDER_HOVER_BELOW_P:
        case SLIDER_HOVER_BELOW_N:
        case SLIDER_HOVER_BELOW:
            *text_pos_var = inner_rect_n.y + inner_rect_n.h - gui_state.slider_text_margin - text_n.y;
            break;
        }
        text_pos.x = SDL_clamp(text_pos.x, inner_rect.x+gui_state.slider_text_margin, inner_rect.x+inner_rect.w-gui_state.slider_text_margin-text_size.x);
        text_pos.y = SDL_clamp(text_pos.y, inner_rect.y+gui_state.slider_text_margin, inner_rect.y+inner_rect.h-gui_state.slider_text_margin-text_size.y);
        SDL_RenderTexture(renderer, text_texture, NULL, &(SDL_FRect){ text_pos.x, text_pos.y, text_texture->w * scale, text_texture->h * scale });
        SDL_DestroyTexture(text_texture);
    }
    return is_pressed;
}

bool Gui_Slider(SDL_Renderer *renderer, SDL_FRect rect, const char* text_min, const char* text_hover, const char* text_max, Slider_Hover_Text_Pos hover_text_pos, float *value, float *hover_value, float min, float max, Slider_Orientation orientation)
{
    return Gui_Slider_State(renderer, rect, text_min, text_hover, text_max, hover_text_pos, value, hover_value, min, max, orientation, gui_state);
}
