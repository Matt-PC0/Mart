![logo](https://github.com/Matt-PC0/Mart/blob/main/logo.png?raw=true)
# Mart
Mart is a minimalist client for MPD focusing on displaying cover art as a compainion client.

# Screenshots
## (toggleable) gui
![gui](https://github.com/Matt-PC0/Mart/blob/main/screen-shots/gui.GIF?raw=true)
## Mart with ncmpcpp
![mart_ncmpcpp](https://github.com/user-attachments/assets/1d6a19f4-d558-4fb3-a053-7949c3c8bad6)

# Building
## Dependancies
* SDL3
* SDL3_image
* SDL3_ttf
* libmpdclient
## locally
```
make release
```
## Installing
```
make install
```
The Makefile provided will install to the Mart binary /usr/local/bin and the man pages to /usr/local/man/man[15] by default.
This is changeable with BINDIR and MANDIR at the head of the file.

# Documentation
Mart comes with groff formated man pages, after `make install` running `man mart` and `man mart.toml` should provide documentation.
# configuration
see `man mart.toml` or the example mart.toml in the root directory.
