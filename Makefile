#
# Copyright [2026] [KamilMalicki]
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Compiler and Flags
CC      = gcc
CFLAGS  = -O2 -Wall -Wextra
LIBS    = -lX11
PREFIX  = /usr/local
BINDIR  = $(DESTDIR)$(PREFIX)/bin

# Identify the real user if running via sudo
REAL_USER = $(shell if [ -n "$$SUDO_USER" ]; then echo "$$SUDO_USER"; else echo "$$USER"; fi)
REAL_HOME = $(shell getent passwd $(REAL_USER) | cut -d: -f6)
SCRIPT_PATH = $(REAL_HOME)/.autoconfigscriptmicwm

.PHONY: all clean install uninstall

# Default target
all: micwm

# Compilation
micwm: main.c config.h
	@printf "  [ CC ]  $@\n"
	$(CC) $(CFLAGS) main.c -o $@ $(LIBS)

# Installation
install: all
	@printf "  [ INSTALL ]  Installing to $(BINDIR)\n"
	install -Dm755 micwm $(BINDIR)/micwm
	
	@printf "  [ CONFIG ]   Checking autostart script...\n"
	@if [ ! -f "$(SCRIPT_PATH)" ]; then \
		printf "-> Creating default autostart script at $(SCRIPT_PATH)\n"; \
		printf '%s\n' "$$AUTOSTART_CONTENT" > "$(SCRIPT_PATH)"; \
		chown $(REAL_USER):$(REAL_USER) "$(SCRIPT_PATH)"; \
		chmod +x "$(SCRIPT_PATH)"; \
	else \
		printf "-> Skrypt $(SCRIPT_PATH) already exists, skipping.\n"; \
	fi

# Definition of the autostart script content
define AUTOSTART_CONTENT
#!/bin/bash
# Autostart configuration for micwm

# Uncomment to set wallpaper (requires feh)
# feh --bg-scale /path/to/wallpaper.jpg &

# Dynamic bar with RAM and clock
while true; do
    RAM=$$(free -m | awk '/Mem/ {print $$3}')
    TIME=$$(date '+%H:%M:%S')
    xsetroot -name " RAM: $$RAM MB | Time: $$TIME "
    sleep 1
done &
endef
export AUTOSTART_CONTENT

# Cleanup
clean:
	@printf "  [ CLEAN ]  Removing binary\n"
	rm -f micwm

# Uninstallation
uninstall:
	@printf "  [ UNINSTALL ]  Removing binary from $(BINDIR)\n"
	rm -f $(BINDIR)/micwm
	@printf "  [ NOTE ]  Config file $(SCRIPT_PATH) preserved.\n"
