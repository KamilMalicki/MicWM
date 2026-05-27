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

# Strictly POSIX-compliant macro expansion avoiding multiline shell constructs
# Evaluates effective user ID identity through standard parameter expansion
REAL_USER := $(shell b=$${SUDO_USER:-$$(id -nu)}; echo "$$b")
REAL_HOME := $(shell eval echo "~$(REAL_USER)")

.PHONY: all install uninstall clean

all: micwm

# Compilation pipeline utilizing standard implicit macros
# CFLAGS handles architecture-specific heavy optimization (-march=native)
# CPPFLAGS enforces compliant POSIX namespace visibility APIs during preprocessing
micwm: main.c config.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) main.c -o micwm $(LIBS)

install: all
	install -m 755 micwm "$(DESTDIR)$(PREFIX)/bin/micwm"
	@echo "Konfiguracja profesjonalnego srodowiska w $(REAL_HOME)..."
	@mkdir -p "$(REAL_HOME)/.config/picom"
	@mkdir -p "$(REAL_HOME)/.micwm"
	@echo 'shadow = false;' > "$(REAL_HOME)/.config/picom/micwm.conf"
	@echo 'corner-radius = 8;' >> "$(REAL_HOME)/.config/picom/micwm.conf"
	@echo 'active-opacity = 0.95;' >> "$(REAL_HOME)/.config/picom/micwm.conf"
	@echo 'inactive-opacity = 0.85;' >> "$(REAL_HOME)/.config/picom/micwm.conf"
	@echo 'frame-opacity = 1.0;' >> "$(REAL_HOME)/.config/picom/micwm.conf"
	
	@echo '# --- SILNIK GLX (Fix na rozrywanie obrazu i ghosty tła) ---' >> "$(REAL_HOME)/.config/picom/micwm.conf"
	@echo 'backend = "glx";' >> "$(REAL_HOME)/.config/picom/micwm.conf"
	@echo 'glx-no-stencil = true;' >> "$(REAL_HOME)/.config/picom/micwm.conf"
	@echo 'glx-copy-from-front = false;' >> "$(REAL_HOME)/.config/picom/micwm.conf"
	@echo 'use-damage = true;' >> "$(REAL_HOME)/.config/picom/micwm.conf"
	@echo 'vsync = true;' >> "$(REAL_HOME)/.config/picom/micwm.conf"
	
	@echo '# --- Niskokosztowe Animacje GPU ---' >> "$(REAL_HOME)/.config/picom/micwm.conf"
	@echo 'fading = false;' >> "$(REAL_HOME)/.config/picom/micwm.conf"
	
	@echo "#!/bin/sh" > "$(REAL_HOME)/.autoconfigscriptmicwm"
	@echo "" >> "$(REAL_HOME)/.autoconfigscriptmicwm"
	@echo "killall -q polybar" >> "$(REAL_HOME)/.autoconfigscriptmicwm"
	@echo "killall -q picom" >> "$(REAL_HOME)/.autoconfigscriptmicwm"
	@echo "sleep 1" >> "$(REAL_HOME)/.autoconfigscriptmicwm"
	@echo "" >> "$(REAL_HOME)/.autoconfigscriptmicwm"
	@echo "# 1. Kompozytor z obsluga animacji na silniku OpenGL (GLX)" >> "$(REAL_HOME)/.autoconfigscriptmicwm"
	@echo "picom --config $(REAL_HOME)/.config/picom/micwm.conf -b &" >> "$(REAL_HOME)/.autoconfigscriptmicwm"
	@echo "" >> "$(REAL_HOME)/.autoconfigscriptmicwm"
	@echo "# 2. Tapeta" >> "$(REAL_HOME)/.autoconfigscriptmicwm"
	@echo "if [ -f \"$(REAL_HOME)/.micwm/wallpaper.png\" ]; then feh --bg-scale \"$(REAL_HOME)/.micwm/wallpaper.png\" & fi" >> "$(REAL_HOME)/.autoconfigscriptmicwm"
	@echo "" >> "$(REAL_HOME)/.autoconfigscriptmicwm"
	@echo "# 3. Polybar na dole" >> "$(REAL_HOME)/.autoconfigscriptmicwm"
	@echo "polybar example &" >> "$(REAL_HOME)/.autoconfigscriptmicwm"
	
	@chown "$(REAL_USER)" "$(REAL_HOME)/.config/picom"
	@chown "$(REAL_USER)" "$(REAL_HOME)/.config/picom/micwm.conf"
	@chown "$(REAL_USER)" "$(REAL_HOME)/.micwm"
	@chown "$(REAL_USER)" "$(REAL_HOME)/.autoconfigscriptmicwm"
	@chmod 755 "$(REAL_HOME)/.autoconfigscriptmicwm"
	@echo "-> Zakonczono instalacje z sukcesem."

uninstall:
	rm -f "$(DESTDIR)$(PREFIX)/bin/micwm"
	rm -f "$(REAL_HOME)/.autoconfigscriptmicwm"
	rm -f "$(REAL_HOME)/.config/picom/micwm.conf"

clean:
	rm -f micwm