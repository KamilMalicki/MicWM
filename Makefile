# ==============================================================================
# micwm - POSIX compliant Makefile for X11 Window Manager (Classic Output Style)
# Reference Standard: IEEE Std 1003.1-2017 (POSIX.1)
# ==============================================================================

# Custom overriding of standard POSIX utilities (Compiler independent)
CC       ?= gcc
CFLAGS   ?= -O3 -march=native -pipe -Wall -Wextra -pedantic
CPPFLAGS ?= -D_POSIX_C_SOURCE=200809L
LDFLAGS  ?=
LIBS     = -lX11
PREFIX   ?= /usr/local

# Detekcja użytkownika i jego katalogu domowego
REAL_USER = $(shell if [ -n "$$SUDO_USER" ]; then echo "$$SUDO_USER"; else echo "$$USER"; fi)
REAL_HOME = $(shell getent passwd $(REAL_USER) | cut -d: -f6)

.PHONY: all install uninstall clean

all: micwm

# Standardowa linia kompilacji: Preprocesor -> CFlags -> Linker
micwm: main.c config.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) main.c -o micwm $(LIBS)

install: all
	install -Dm755 micwm $(DESTDIR)$(PREFIX)/bin/micwm
	@echo "Sprawdzam skrypt autostartu w $(REAL_HOME)..."
	@if [ ! -f "$(REAL_HOME)/.autoconfigscriptmicwm" ]; then \
		echo "#!/bin/bash" > "$(REAL_HOME)/.autoconfigscriptmicwm"; \
		echo "" >> "$(REAL_HOME)/.autoconfigscriptmicwm"; \
		echo "# MicWM Autostart" >> "$(REAL_HOME)/.autoconfigscriptmicwm"; \
		echo "# feh --bg-scale /path/to/wallpaper.jpg &" >> "$(REAL_HOME)/.autoconfigscriptmicwm"; \
		echo "" >> "$(REAL_HOME)/.autoconfigscriptmicwm"; \
		echo "while true; do" >> "$(REAL_HOME)/.autoconfigscriptmicwm"; \
		echo "    RAM=\$$(free -m | awk '/Mem/ {print \$$3}')" >> "$(REAL_HOME)/.autoconfigscriptmicwm"; \
		echo "    CZAS=\$$(date '+%H:%M:%S')" >> "$(REAL_HOME)/.autoconfigscriptmicwm"; \
		echo "    xsetroot -name \" RAM: \$$RAM MB | Czas: \$$CZAS \"" >> "$(REAL_HOME)/.autoconfigscriptmicwm"; \
		echo "    sleep 1" >> "$(REAL_HOME)/.autoconfigscriptmicwm"; \
		echo "done &" >> "$(REAL_HOME)/.autoconfigscriptmicwm"; \
		chown $(REAL_USER):$(REAL_USER) "$(REAL_HOME)/.autoconfigscriptmicwm"; \
		chmod +x "$(REAL_HOME)/.autoconfigscriptmicwm"; \
		echo "-> Utworzono: $(REAL_HOME)/.autoconfigscriptmicwm"; \
	else \
		echo "-> Skrypt juz istnieje."; \
	fi

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/micwm
	@echo "-> Binarka usunieta. Skrypt w $(REAL_HOME) pozostal nienaruszony."

clean:
	rm -f micwm