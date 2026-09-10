MicWM (Minimalist C Window Manager)

MicWM is a lightweight window manager for the X11 system, written in C using the Xlib library. The project focuses on minimal resource usage and total user control.

Features

Memory footprint around 2 MB RAM.

Configured via the config.h file (requires recompilation).

Status bar updated via xsetroot (e.g., clock, RAM usage).

Custom autostart script (~/.autoconfigscriptmicwm).

Window termination using XKillClient.

Customizable window border color.

Function key support for adjusting volume and brightness.

Window locking and unlocking feature.

Requirements and Installation

Installing dependencies (Debian/Ubuntu):
sudo apt install libx11-dev gcc make feh x11-xserver-utils xterm brightnessctl alsa-utils

Compilation and installation:
make
sudo make install

The installer automatically creates a template autostart script in your home directory.

Keybindings

Super + Enter - Launch terminal (st)
Super + Q - Close active window
Super + Shift + F - Fullscreen mode
Super + Shift + W - Windowed mode (800x600, centered)
Super + Shift + Q - Exit MicWM
Super + Left Click - Move window
Super + Right Click - Resize window
Super + D - Lock / unlock window
Super + S - Auto-arrange windows

Configuration

Appearance: Edit colors, border width, or the default terminal in config.h, then run: make && sudo make install.

Wallpaper and Status: Edit settings inside ~/.autoconfigscriptmicwm.

Usage Notes

Running applications as a regular user from root:
sudo -u username program_name

Allowing non-root users to run X11:
sudo dpkg-reconfigure xserver-xorg-legacy
(select "Anybody")

Starting a session from TTY:
xinit /usr/local/bin/micwm -- :1

License

The code is open for personal compilation and modification. Redistribution under your own name is prohibited.

Kamil Malicki
