![logo](https://github.com/Matt-PC0/Mart/blob/main/logo.png?raw=true)
# Mart
Mart is a minimalist client for [MPD](https://www.musicpd.org/) focusing on displaying cover art as a compainion client.

# Screenshots
## (toggleable) gui
![gui](https://github.com/Matt-PC0/Mart/blob/main/screen-shots/gui.GIF?raw=true)
## Mart with ncmpcpp
![mart_ncmpcpp](https://github.com/user-attachments/assets/1d6a19f4-d558-4fb3-a053-7949c3c8bad6)

# Building
The build system is very simple to non-existant so be warned.
## Dependancies
* [SDL3](https://github.com/libsdl-org/SDL)
* [SDL3_image](https://github.com/libsdl-org/SDL_image)
* [SDL3_ttf](https://github.com/libsdl-org/SDL_ttf)
* [libmpdclient](https://www.musicpd.org/libs/libmpdclient/)
## locally
```
make release
```
## Installing
```
make install
```
The [Makefile](https://github.com/Matt-PC0/Mart/blob/main/Makefile) provided will install to the Mart binary /usr/local/bin and the man pages to /usr/local/man/man[15] by default.
This is changeable with BINDIR and MANDIR at the head of the file.

# Documentation
Mart comes with groff formated man pages, after `make install` running `man mart` and `man mart.toml` should provide documentation.
# configuration
see `man mart.toml` or the example [mart.toml](https://github.com/Matt-PC0/Mart/blob/main/mart.toml) in the root directory.
