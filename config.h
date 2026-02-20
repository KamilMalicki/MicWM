// config.h - tu ustawiam wszystkie duperele zeby nie grzebac w glownym kodzie

// kolory ramek (zapisane w HEX)
#define KOLOR_ZWYKLY  0xffffff  // biala ramka jak okno sobie lezy
#define KOLOR_FOCUS   0xffffff  // wsciekly zolty jak w nie klikne/najade

// rozmiary
#define GRUBOSC_RAMKI 4         // jaka gruba ramka w pikselach
#define MIN_ROZMIAR   10        // do ilu pikseli moge scisnac okno (zeby nie zepsuc X11)

// pasek na gorze
#define WYSOKOSC_PASKA 20       // ile pikseli zre pasek
#define TEKST_NA_PASKU " MicWM - Czekam na xsetroot... " // to sie wyswietli tylko do momentu odpalenia skryptu z zegarem

// jaki terminal odpalam pod Alt+T
#define MOJ_TERMINAL "st"

// skrypt startowy z tapeta i paskiem (zeby dalo sie zmienic bez grzebania w silniku)
#define AUTOSTART_SCRIPT "bash $HOME/.autoconfigscriptmicwm &"