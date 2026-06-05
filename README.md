# 🪟 MicWM (Minimalist C Window Manager)

**MicWM** is an ultra-lightweight, spartan window manager written in pure C using the Xlib library. It’s designed for speed, zero bloat, and total user control.

> "Because RAM is for processing, not for desktop animations."

### 🚀 Features

* **Featherweight:** Memory footprint around **2 MB**.
* **Suckless Philosophy:** Configured via `config.h` (requires recompilation).
* **Dynamic Status Bar:** Real-time updates (Clock, RAM, etc.) via `xsetroot`.
* **Custom Autostart:** Dedicated `~/.autoconfigscriptmicwm` script for wallpapers and startup apps.
* **Brutal Management:** Uses `XKillClient` for aggressive process termination to save resources.
* **The Glow:** A border that can have a color selected from the `config.h` file.
* **Option to change the sound level and brightness:** As in other systems, use the f keys.
* **Blocking and unblocking windows:** Lock and unlock apps to remove the cursor from apps that grab the cursor or to simply keep it out of the way.

### 🛠️ Installation

1. **Dependencies** Install X11 libraries and build tools:
```bash
sudo apt install libx11-dev gcc make feh x11-xserver-utils xterm brightnessctl alsa-utils

```

2. **Build and Install** 
```bash
make
sudo make install

```


*The installer automatically creates a template autostart script in your home directory.*



### ⌨️ Keybindings

| Keybind | Action |
| :--- | :--- |
| **Super + T** | Launch Terminal (`st`) |
| **Super + Q** | Kill active window (Brutal/Instant) |
| **Super + Shift + F** | Fullscreen Mode (No borders) |
| **Super + Shift + W** | Windowed Mode (800x600, Centered) |
| **Super + Shift + Q** | **Exit MicWM** (Return to TTY) |
| **Super + Left Click** | Move Window |
| **Super + Right Click** | Resize Window |
| **Super + D**| Lock and Unlock Window |



### ⚙️ Configuration & Autostart

* **Appearance:** Edit `config.h` to change colors (HEX), border width, or the default terminal. Run `make && sudo make install` to apply.
* **Wallpaper & Status:** Edit `~/.autoconfigscriptmicwm`. Uncomment the `feh` line and set the path to your image.

---

### 💡 Pro-Tips

#### 1. Running apps as a specific user
If you are logged in as **root** (e.g., in a TTY) but want to launch applications that belong to your regular user (to keep config files clean and avoid permission issues), use the following command inside your MicWM terminal:

```bash
sudo -u yourusername program_name

```

*Example:* `sudo -u oem xterm` or `sudo -u oem bash /home/oem/script.sh`.

#### 2. Fix "Permission Denied" on TTY

If you can't start the GUI without `sudo`, allow non-root users to run the X server:

```bash
sudo dpkg-reconfigure xserver-xorg-legacy
# Select "Anybody"

```

#### 3. Starting the Session

Launch from a clean TTY (e.g., `Ctrl+Alt+F3`):

```bash
xinit /usr/local/bin/micwm -- :1

```

---

### 📜 License

The GUI is open source, so you can compile it any way you like, but it is not for distribution under your own name.

**With respect, Kamil Malicki**
