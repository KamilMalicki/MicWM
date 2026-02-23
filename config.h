// config.h - główny plik konfiguracyjny MicWM
// Aby zastosować zmiany, zapisz plik i przebuduj menedżer (make && sudo make install)

// --- KLAWISZ MODYFIKATORA ---
// Mod1Mask = Alt
// Mod4Mask = Super (klawisz Windows)
#define MOD_KEY       Mod4Mask

// --- WYGLĄD I KOLORY ---
#define KOLOR_ZWYKLY  0x1f1f1f  // Kolor ramki nieaktywnego okna (HEX)
#define KOLOR_FOCUS   0xffffff  // Kolor ramki aktywnego okna (HEX)

// --- WYMIARY ---
#define GRUBOSC_RAMKI 4         // Grubość obramowania
#define MIN_ROZMIAR   10        // Minimalny rozmiar okna

// --- PASEK STATUSU ---
#define WYSOKOSC_PASKA 20       // Wysokość paska w pikselach
#define TEKST_NA_PASKU " MicWM " // Tekst startowy

// --- APLIKACJE I SKRYPTY ---
#define MOJ_TERMINAL "st"       // Domyślny terminal
#define AUTOSTART_SCRIPT "bash $HOME/.autoconfigscriptmicwm &" // Skrypt autostartu