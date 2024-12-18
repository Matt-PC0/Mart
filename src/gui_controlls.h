
#ifndef MART_GUI_CONTROLLS_H
#define MART_GUI_CONTROLLS_H

#include <mpd/client.h>

void gui_Player_Controlls(struct mpd_connection *mpd, bool show_gui);
void Gui_Title(struct mpd_connection *mpd, bool show_gui);
void Gui_Volume_Controll(struct mpd_connection *mpd, bool show_gui);
void Gui_Mode_Controlls(struct mpd_connection *mpd, bool show_gui);

#endif //ifndef MART_GUI_CONTROLLS_H
