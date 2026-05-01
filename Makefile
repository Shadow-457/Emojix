CC = gcc
CFLAGS = -Wall -Wextra -O2
LIBS = -lX11 -lXtst
PREFIX ?= $(HOME)/.local

all: emojix

emojix: main.c emojis.h
	$(CC) $(CFLAGS) -o emojix main.c $(LIBS)

emojis.h:
	@echo "Checking emojis.h... if missing, run python3 generate.py <json> emojis.h"

install: emojix
	install -D -m 755 emojix $(PREFIX)/bin/emojix
	mkdir -p $(HOME)/.config/autostart
	mkdir -p $(HOME)/.local/share/applications
	mkdir -p $(HOME)/.local/share/icons/hicolor/scalable/apps
	cp logo.svg $(HOME)/.local/share/icons/hicolor/scalable/apps/emojix.svg
	printf "[Desktop Entry]\nName=Emojix\nComment=Global Emoji Expander Background Service\nExec=sh -c 'pkill emojix; $(PREFIX)/bin/emojix'\nIcon=emojix\nTerminal=false\nType=Application\nCategories=Utility;\n" > $(HOME)/.local/share/applications/emojix.desktop
	printf "[Desktop Entry]\nName=Emojix\nComment=Global Emoji Expander\nExec=$(PREFIX)/bin/emojix\nTerminal=false\nType=Application\n" > $(HOME)/.config/autostart/emojix.desktop
	gtk-update-icon-cache -f -t $(HOME)/.local/share/icons/hicolor || true
	update-desktop-database $(HOME)/.local/share/applications || true
	@echo "✅ Emojix successfully installed!"
	@echo "You can now launch it directly from your Start Menu!"

uninstall:
	rm -f $(PREFIX)/bin/emojix
	rm -f $(HOME)/.config/autostart/emojix.desktop
	rm -f $(HOME)/.local/share/applications/emojix.desktop
	rm -f $(HOME)/.local/share/icons/hicolor/scalable/apps/emojix.svg
	@echo "🗑️ Emojix successfully uninstalled."

clean:
	rm -f emojix
