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
	@echo "[Desktop Entry]\nName=Emojix\nComment=Global Emoji Expander\nExec=$(PREFIX)/bin/emojix\nTerminal=false\nType=Application" > $(HOME)/.config/autostart/emojix.desktop
	@echo "✅ Emojix successfully installed to $(PREFIX)/bin/emojix"
	@echo "✅ Autostart entry added to ~/.config/autostart/emojix.desktop"
	@echo "\nRun 'emojix &' to start it right now!"

uninstall:
	rm -f $(PREFIX)/bin/emojix
	rm -f $(HOME)/.config/autostart/emojix.desktop
	@echo "🗑️ Emojix successfully uninstalled."

clean:
	rm -f emojix
