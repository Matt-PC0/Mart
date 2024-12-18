#ifndef MART_IMGUI
#define MART_IMGUI

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

typedef struct{
    SDL_Color text;
    SDL_Color background;
    SDL_Color border;
} Gui_Colour;

typedef enum
{
    SLIDER_HOVER_MID       = 0,
    SLIDER_HOVER_FOLLOW    = 1,

    //as seen in LTR or RLT its rotated to be right and left in TTB and BBT
    SLIDER_HOVER_ABOVE     = 2,
    SLIDER_HOVER_BELOW     = 3,

    _SLIDER_HOVER_P_START_ = 4,
    SLIDER_HOVER_MID_P     = 4,
    SLIDER_HOVER_FOLLOW_P  = 5,
    SLIDER_HOVER_ABOVE_P   = 6,
    SLIDER_HOVER_BELOW_P   = 7,
    _SLIDER_HOVER_P_END_   = 7,

    _SLIDER_HOVER_N_START_ = 8,
    SLIDER_HOVER_MID_N     = 8,
    SLIDER_HOVER_FOLLOW_N  = 9,
    SLIDER_HOVER_ABOVE_N   = 10,
    SLIDER_HOVER_BELOW_N   = 11,
    _SLIDER_HOVER_N_END_   = 11,
} Slider_Hover_Text_Pos;

typedef struct{
    float slider_text_margin;

    float border_size;

    float conf_font_size;
    float conf_title_font_size;

    TTF_Font  *font;
    float font_point_size;
    float font_size;
    float title_font_size;

    Gui_Colour active_col;
    Gui_Colour inactive_col;
    Gui_Colour hover_col;
} Gui_State;
extern const Gui_State gui_state_default;
extern Gui_State gui_state;

typedef enum
{
    SLIDER_LTR,
    SLIDER_RTL,
    SLIDER_TTB,
    SLIDER_BTT,
} Slider_Orientation;

bool Gui_Button_State(SDL_Renderer *renderer, SDL_FRect rect, const char* text, Gui_State state);
bool Gui_Button(SDL_Renderer *renderer, SDL_FRect rect, const char* text);
bool Gui_Toggle_State(SDL_Renderer *renderer, SDL_FRect rect, const char* text, bool *toggle, Gui_State state);
bool Gui_Toggle(SDL_Renderer *renderer, SDL_FRect rect, const char* text, bool *toggle);
bool Gui_Slider_State(SDL_Renderer *renderer, SDL_FRect rect, const char* text_min, const char *text_hover, const char* text_max, Slider_Hover_Text_Pos hover_text_pos, float *value, float *hover_value, float min, float max, Slider_Orientation orientation , Gui_State state);
bool Gui_Slider(SDL_Renderer *renderer, SDL_FRect rect, const char* text_min, const char* text_hover, const char* text_max, Slider_Hover_Text_Pos hover_text_pos, float *value, float *hover_value, float min, float max, Slider_Orientation orientation);

#endif //ifndef MART_IMGUI
