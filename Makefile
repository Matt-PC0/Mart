BINDIR=/usr/local/bin
BINNAME=mart

MANDIR=/usr/local/man

TOML_C=tomlc99/toml.c
TOML_H=tomlc99/

STB_H=stb/

MART_C=src/main.c

LIBS= -lmpdclient -lm -lSDL3 -lSDL3_ttf -lSDL3_image
INCS= -I$(TOML_H) -I$(STB_H) -Iembeds/
CFLAGS := --std=c99 $(LIBS) $(INCS)


debug: $(MART_C) $(TOML_C) $(TOML_H)
	cc $(MART_C) $(TOML_C) $(CFLAGS) -ggdb -o$(BINNAME)

release: $(MART_C) $(TOML_C) $(TOML_H)
	cc $(MART_C) $(TOML_C) $(CFLAGS) -O3 -o $(BINNAME)

run: debug
	./mart

valgrind: debug
	valgrind --leak-check=yes ./mart

install: release mart
	#install binary
	mkdir -p $(BINDIR)
	cp -f mart $(BINDIR)/$(BINNAME)
	chmod 755 $(BINDIR)/$(BINNAME)
	#install manpages
	mkdir -p $(MANDIR)/man1
	cp -f docs/mart.1 $(MANDIR)/man1/mart.1
	mkdir -p $(MANDIR)/man5
	cp -f docs/mart.toml.5 $(MANDIR)/man5/mart.toml.5

uninstall:
	rm -f $(BINDIR)/$(BINNAME)
	rm -f $(MANDIR)/man1/mart.1
	cp -f $(MANDIR)/man5/mart.toml.5

