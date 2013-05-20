#common
CFLAGS=-Wall -ggdb -m32 -D_GNU_SOURCE
SRC=src/nibbles.c src/cexplode.c src/stringfilters.c src/bshandler.c src/udp_handler.c src/tgt_commander.c src/syscomform.c src/displayhandler.c src/definitionfinder.c src/common.c src/shitemsgparser.c src/msgloadermenu.c
LDFLAGS=
#commonstatic
CFLAGS_STATIC=-DNCURSES_STATIC
LDFLAGS_STATIC=-static


#linux
CC=gcc
LIBS=-lpthread -lncurses -lform -lpanel -lmenu
TGT=bin/nibbles
TGT_STATIC=bin/nibbles_static

#windows
#I did not find menu/panel libs for windows => windows no longer supported
#WINCC=i686-pc-mingw32-gcc
#WINCFLAGS=-Iwin
#WINLIBS=-Lwin/ -lpdcurses -lws2_32 -lpthread
#WINDEFINES=-D_WINDOWS_
#staticwin
#WINLIBS_STATIC=-Lwin/ -lpdcurses
#WINLIBS_DYN=-lws2_32 -lpthread
#WINTGT=bin/nibbles.exe
#WINTGT_STATIC=bin/nibbles_static.exe


all: nibbles
man: nibbles.8.gz

nibbles: $(SRC)
	$(CC) $(CFLAGS) -o $(TGT) $(SRC) $(LDFLAGS) $(LIBS)

help:
	@echo 'Targets: nibbles, testkeys, nibbles_static, install, maninstall, clean'

testkeys: src/testkeys.c
	$(CC) $(CFLAGS) src/testkeys.c -o bin/testkeys  $(LIBS)

nibbles_static: $(SRC)
	$(CC) $(CFLAGS) $(CFLAGS_STATIC) -o $(TGT_STATIC) $(SRC) $(LDFLAGS) $(LDFLAGS_STATIC) $(LIBS)

#wibbles: $(SRC)
#	$(WINCC) $(WINDEFINES) $(CFLAGS) $(WINCFLAGS) -o $(WINTGT) $(SRC) $(LDFLAGS) $(WINLIBS)

#wibbles_static:
#	$(WINCC) $(WINDEFINES) $(CFLAGS_STATIC) $(CFLAGS) $(WINCFLAGS) -o $(WINTGT_STATIC) $(SRC) $(LDFLAGS) -Wl,-static $(WINLIBS_STATIC) -Wl,-Bdynamic $(WINLIBS_DYN)

install: $(TGT) maninstall
	cp $(TGT) /usr/bin/.

maninstall: nibbles.8.gz
	mv nibbles.8.gz /usr/share/man/man8/.
	@echo 'man pages installed to /usr/share/man/man8'
	@echo 'consider running mandb or makewhatis to update apropos database'

clean:
	rm -rf $(TGT)
	rm -rf $(WINTGT)
	rm -rf bin/testkeys
	rm -rf $(WINTGT_STATIC)
	rm -rf $(TGT_STATIC)


nibbles.8.gz: man/nibbles.8
	cp man/nibbles.8 nibbles.8
	gzip nibbles.8
