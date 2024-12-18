
#include <mpd/client.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <stdlib.h>
//#include "stb_image.h"

#include "album-art.h"
#include "config.h"

Album_Art_Error Get_Album_Art_Type(struct mpd_connection* mpd, struct mpd_song* song, void *dst_buffer[], size_t *dst_buffer_size, bool embedded)
{

#define MPD_CHUNK_SIZE 8192
#define MPD_SIZE_LIMIT 5242880

    if (mpd == NULL || song == NULL || dst_buffer == NULL || dst_buffer_size == NULL)
        return ART_ERR_NULL_ARG;

    const char * uri = mpd_song_get_uri(song);

    int recv_size = 0;
    unsigned offset = 0;

    size_t buffer_size = MPD_CHUNK_SIZE;
    void *buffer = SDL_malloc(buffer_size);


    //while ( (recv_size = ((embedded) ? mpd_run_readpicture(mpd, uri , offset, buffer+offset, MPD_CHUNK_SIZE) : mpd_run_albumart(mpd, uri , offset, buffer+offset, MPD_CHUNK_SIZE))) > 0)
    while (true)
    {
        if (embedded)
            recv_size = mpd_run_readpicture(mpd, uri , offset, buffer+offset, MPD_CHUNK_SIZE);
        else 
            recv_size = mpd_run_albumart(mpd, uri , offset, buffer+offset, MPD_CHUNK_SIZE);

        if (recv_size == 0) // 0 done reading
        {
            break;
        }
        if (recv_size < 0) // -1 error
        {
            SDL_Log("album art ran into an error\n");
            SDL_free(buffer);
            return ART_ERR_NEGATIVE;
        }

        buffer_size += MPD_CHUNK_SIZE;
        buffer = SDL_realloc(buffer, buffer_size);
        offset += (unsigned)recv_size;

        if (offset >= MPD_SIZE_LIMIT - MPD_CHUNK_SIZE)
        {
            SDL_Log("album art is too large for mpd > 5MB\n");
            SDL_free(buffer);
            return ART_ERR_TOO_BIG;
        }
    }
    *dst_buffer      = buffer;
    *dst_buffer_size = buffer_size;
    return ART_ERR_NONE;
}
Album_Art_Error Get_Album_Art(struct mpd_connection* mpd, struct mpd_song* song, void *dst_buffer[], size_t *dst_buffer_size)
{
    Album_Art_Error err = ART_ERR_NONE;
    err = Get_Album_Art_Type(mpd, song, dst_buffer, dst_buffer_size, true);
    if (err != ART_ERR_NONE)
        err = Get_Album_Art_Type(mpd, song, dst_buffer, dst_buffer_size, false);
    return err;
}


/*
  Crudely manually reading the file headers becuase mpd_run_*() does not return mime type
*/
const char *Get_File_Type(char *buffer, size_t buffer_size)
{
    if (buffer_size < 10)
    {
        SDL_Log("buffer_size too small for call to Get_File_Type\n");
        exit(2);
    }

    bool is_type_found;

    is_type_found = true;
    for (int i = 0; _PNG_HDR[i] != '\0'; i += 1) {
        if (buffer[_PNG_I + i] != _PNG_HDR[i])
        {
            is_type_found = false;
            break;
        }
    }
    if (is_type_found)
        return _PNG_EXT;

    is_type_found = true;
    for (int i = 0; _EXIF_HDR[i] != '\0'; i += 1) {
        if (buffer[_EXIF_I + i] != _EXIF_HDR[i])
        {
            is_type_found = false;
            break;
        }
    }
    if (is_type_found)
        return _JPG_EXT;

    is_type_found = true;
    for (int i = 0; _JFIF_HDR[i] != '\0'; i += 1) {
        if (buffer[_JFIF_I + i] != _JFIF_HDR[i])
        {
            is_type_found = false;
            break;
        }
    }
    if (is_type_found)
        return _JPG_EXT;

    return NULL;
}

SDL_Texture *Get_Album_Texture_Default( SDL_Renderer *renderer, const char *file_path)
{
    SDL_Surface *cover_art_surface = IMG_Load(file_path);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, cover_art_surface);
    SDL_DestroySurface(cover_art_surface);

    return texture;
}

SDL_Texture *Get_Album_Texture(SDL_Renderer *renderer, struct mpd_connection* mpd, struct mpd_song* song)
{
    SDL_Texture *texture = NULL;
    void * buffer = NULL;
    size_t buffer_size;
    Album_Art_Error err = Get_Album_Art(mpd, song, &buffer, &buffer_size);
    if (err == ART_ERR_NEGATIVE) //probaly does not have embedded album art, which would be fine but mpd shits itself so geuss ill die
    {
        // TODO: attempt to reconnect to mpd server
        SDL_Log("failed to get album art, there likely is none. Due to a bug the mpd connection is now mangled; attemping to reconnect\n");
        mpd_connection_free(mpd);
        mpd = mpd_connection_new(NULL, 0, 0);
        if (mpd == NULL)
        {
            SDL_Log("Couldnt connected to mpd!\n");
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Couldnt connected to mpd!\n", "Couldnt connected to mpd!\n", NULL);
            exit(0);
        }
        //exit(0);
    }

    if (buffer != NULL)
    {
        SDL_Surface *cover_art_surface = IMG_Load_IO(SDL_IOFromMem(buffer, buffer_size), true);
        texture = SDL_CreateTextureFromSurface(renderer, cover_art_surface);
        SDL_DestroySurface(cover_art_surface);

        SDL_free(buffer);
    }
    else if (mart_config.placeholder_image)
        texture = Get_Album_Texture_Default(renderer, mart_config.placeholder_image);

    return texture;
}
