/* * micwm - minimalistic window manager config
 * See LICENSE file for copyright and license details.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <X11/Xlib.h>
#include <X11/keysym.h>

/* --- Modifier Key --- */
#define MOD_KEY              Mod4Mask        /* Mod4Mask is the Super/Windows key */

/* --- Appearance & Styling --- */
#define GRUBOSC_RAMKI        2               /* Window border width in pixels */
#define MIN_ROZMIAR          10              /* Minimum window width/height */
#define MARGINES_DOLNY       35              /* Bottom margin in pixels reserved for Polybar */
#define KOLOR_ZWYKLY         0x282c34        /* Dark grey hex color for unfocused borders */
#define KOLOR_FOCUS          0x61afef        /* Light blue hex color for focused borders */

/* --- Core Applications & Autostart --- */
#define MOJ_TERMINAL         "alacritty"
#define MOJ_LAUNCHER         "dmenu_run"
#define MOJ_SCREENSHOT       "maim ~/screenshot-$(date +%s).png"
#define AUTOSTART_SCRIPT     "bash $HOME/.autoconfigscriptmicwm"

/* --- Keybindings Configuration (KeySym references) --- */
#define KLUCZ_TERMINAL       XK_Return       /* Launch terminal */
#define KLUCZ_LAUNCHER       XK_p            /* Launch application menu */
#define KLUCZ_ZAMKNIJ        XK_q            /* Close focused client / quit WM */
#define KLUCZ_LAYOUT         XK_s            /* Switch window layouts */
#define KLUCZ_SCREENSHOT     XK_s            /* Used with Shift for custom screenshot */
#define KLUCZ_PRINT_SCREEN   XK_Print        /* Dedicated screenshot key */
#define KLUCZ_LOCK_FOCUS     XK_d            /* Lock input focus on current client */
#define KLUCZ_FLOATING       XK_space        /* Toggle floating status for window */
#define KLUCZ_FOCUS_NEXT     XK_j            /* Focus next window in stack */
#define KLUCZ_FOCUS_PREV     XK_k            /* Focus previous window in stack */
#define KLUCZ_MASTER_MINUS   XK_h            /* Decrease master area size */
#define KLUCZ_MASTER_PLUS    XK_l            /* Increase master area size */
#define KLUCZ_RESTART        XK_r            /* Hot-restart micwm without losing windows */
#define KLUCZ_MAKSYMALIZUJ   XK_f            /* True fullscreen toggle */
#define KLUCZ_ROZMIAR_W      XK_w            /* Quick-resize window to centered 800x600 */

/* --- Interactive Utilities --- */
#define KLUCZ_GAPS_PLUS      XK_equal        /* Dynamic gaps mixer: increase spacing */
#define KLUCZ_GAPS_MINUS     XK_minus        /* Dynamic gaps mixer: decrease spacing */
#define KLUCZ_POMOCY         XK_u            /* Launch interactive help window */

/* --- Help Window Command --- */
#define POLECENIE_POMOCY     "alacritty --title \"MicWM_Help\" -o font.size=8 -e less $HOME/.micwm/keymapinfo.txt"

#endif /* CONFIG_H */