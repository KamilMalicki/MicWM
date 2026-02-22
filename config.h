// config.h - główny plik konfiguracyjny MicWM
// Aby zastosować zmiany, zapisz plik i przebuduj menedżer (make && sudo make install)

// --- WYGLĄD I KOLORY ---
#define KOLOR_ZWYKLY  0x1f1f1f  // Kolor ramki nieaktywnego okna (HEX). Tu: ciemnoszary.
#define KOLOR_FOCUS   0xffffff  // Kolor ramki aktywnego okna (tego pod myszką). Tu: czysty biały.

// --- WYMIARY ---
#define GRUBOSC_RAMKI 4         // Grubość obramowania dookoła okien (w pikselach).
#define MIN_ROZMIAR   10        // Minimalny rozmiar okna w pikselach (chroni przed crashem X11 przy maksymalnym zmniejszeniu).

// --- PASEK STATUSU ---
#define WYSOKOSC_PASKA 20       // Ile pikseli od góry ekranu zajmuje pasek statusu.
#define TEKST_NA_PASKU " MicWM " // Tekst wyświetlany ułamek sekundy po starcie, zanim skrypt autostartu go nadpisze.

// --- APLIKACJE I SKRYPTY ---
#define MOJ_TERMINAL "st"       // Domyślny emulator terminala odpalany skrótem Alt + T (np. "st", "xterm", "alacritty").
#define AUTOSTART_SCRIPT "bash $HOME/.autoconfigscriptmicwm &" // Skrypt uruchamiany przy starcie (ładuje tapetę, pasek i inne programy w tle).