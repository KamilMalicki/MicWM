# Makefile dla mojego ultra-lekkiego gui

CC = gcc
CFLAGS = -O2 -Wall
LIBS = -lX11
PREFIX = /usr/local

# czarna magia zeby znalezc prawdziwego uzytkownika (nawet jak odpala przez sudo)
REAL_USER = $(shell if [ -n "$$SUDO_USER" ]; then echo "$$SUDO_USER"; else echo "$$USER"; fi)
REAL_HOME = $(shell getent passwd $(REAL_USER) | cut -d: -f6)

all: micwm

micwm: main.c config.h
	$(CC) $(CFLAGS) main.c -o micwm $(LIBS)

install: all
	install -Dm755 micwm $(PREFIX)/bin/micwm
	@echo "Sprawdzam skrypt autostartu..."
	@if [ ! -f "$(REAL_HOME)/.autoconfigscriptmicwm" ]; then \
		echo "#!/bin/bash" > "$(REAL_HOME)/.autoconfigscriptmicwm"; \
		echo "" >> "$(REAL_HOME)/.autoconfigscriptmicwm"; \
		echo "# Odkomentuj to i wpisz sciezke, zeby miec tapete (wymaga feh):" >> "$(REAL_HOME)/.autoconfigscriptmicwm"; \
		echo "# feh --bg-scale /sciezka/do/twojej/tapety.jpg &" >> "$(REAL_HOME)/.autoconfigscriptmicwm"; \
		echo "" >> "$(REAL_HOME)/.autoconfigscriptmicwm"; \
		echo "# Dynamiczny pasek z RAMem i zegarem dla MicWM" >> "$(REAL_HOME)/.autoconfigscriptmicwm"; \
		echo "while true; do" >> "$(REAL_HOME)/.autoconfigscriptmicwm"; \
		echo "    RAM=\$$(free -m | awk '/Mem/ {print \$$3}')" >> "$(REAL_HOME)/.autoconfigscriptmicwm"; \
		echo "    CZAS=\$$(date '+%H:%M:%S')" >> "$(REAL_HOME)/.autoconfigscriptmicwm"; \
		echo "    xsetroot -name \" RAM: \$$RAM MB | Czas: \$$CZAS \"" >> "$(REAL_HOME)/.autoconfigscriptmicwm"; \
		echo "    sleep 1" >> "$(REAL_HOME)/.autoconfigscriptmicwm"; \
		echo "done &" >> "$(REAL_HOME)/.autoconfigscriptmicwm"; \
		chown $(REAL_USER):$(REAL_USER) "$(REAL_HOME)/.autoconfigscriptmicwm"; \
		chmod +x "$(REAL_HOME)/.autoconfigscriptmicwm"; \
		echo "-> ZROBIONE! Stworzono gotowy skrypt w $(REAL_HOME)/.autoconfigscriptmicwm"; \
	else \
		echo "-> Skrypt $(REAL_HOME)/.autoconfigscriptmicwm juz istnieje, zostawiam go w spokoju."; \
	fi

uninstall:
	rm -f $(PREFIX)/bin/micwm
	@echo "-> MicWM usuniety z systemu."
	@echo "-> UWAGA: Twoj skrypt $(REAL_HOME)/.autoconfigscriptmicwm zostal zachowany. Mozesz go usunac recznie."

clean:
	rm -f micwm