#ifndef MART_ALBUM_ART_H
#define MART_ALBUM_ART_H

#include <mpd/client.h>
#include <SDL3/SDL.h>

const char *_PNG_EXT = ".png";
const char *_JPG_EXT = ".jpg";

#define _PNG_I 1
const char *_PNG_HDR = "PNG";
#define _EXIF_I 1
const char *_EXIF_HDR = "Exif";
#define _JFIF_I 6
const char *_JFIF_HDR = "JFIF";

typedef enum Album_Art_Error Album_Art_Error;
enum Album_Art_Error
{
    ART_ERR_NONE,
    ART_ERR_NULL_ARG,
    ART_ERR_NEGATIVE,
    ART_ERR_TOO_BIG,
};


Album_Art_Error Get_Album_Art_Type(struct mpd_connection* mpd, struct mpd_song* song, void *dst_buffer[], size_t *dst_buffer_size, bool embedded);
Album_Art_Error Get_Album_Art(struct mpd_connection* mpd, struct mpd_song* song, void *dst_buffer[], size_t *dst_buffer_size);
const char *Get_File_Type(char *buffer, size_t buffer_size);
SDL_Texture *Get_Album_Texture_Default( SDL_Renderer *renderer, const char *file_path);
SDL_Texture *Get_Album_Texture(SDL_Renderer *renderer, struct mpd_connection* mpd, struct mpd_song* song);


#endif //ifndef MART_ALBUM_ART_H
