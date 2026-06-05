// config.h - Main configuration file for MicWM
// To apply changes, save the file and rebuild (make && sudo make install)

// --- MODIFIER KEY ---
// Mod1Mask = Alt
// Mod4Mask = Super (Windows Key)
#define MOD_KEY       Mod4Mask

// --- APPEARANCE & COLORS ---
#define KOLOR_ZWYKLY  0x1f1f1f  // Border color for inactive windows (HEX)
#define KOLOR_FOCUS   0xffffff  // Border color for active window (HEX)

// --- DIMENSIONS ---
#define GRUBOSC_RAMKI 4         // Border thickness in pixels
#define MIN_ROZMIAR   10        // Minimum window size

// --- STATUS BAR ---
#define WYSOKOSC_PASKA 20       // Bar height in pixels
#define TEKST_NA_PASKU " MicWM " // Default status text

// --- APPLICATIONS & SCRIPTS ---
#define MOJ_TERMINAL "st"       // Default terminal emulator
#define AUTOSTART_SCRIPT "bash $HOME/.autoconfigscriptmicwm &" // Autostart script

#define LANGUAGE "PL"