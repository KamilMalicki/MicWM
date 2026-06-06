/* config.h */
#ifndef CONFIG_H
#define CONFIG_H

#include <X11/Xlib.h>
#include <X11/keysym.h>

/* --- Modifier Key --- */
#define MOD_KEY              Mod4Mask        /* Klawisz Super / Windows */

/* --- Appearance & Styling --- */
#define GRUBOSC_RAMKI        2               
#define MIN_ROZMIAR          10              
#define MARGINES_GORNY       35            
#define KOLOR_ZWYKLY         0x282c34        
#define KOLOR_FOCUS          0x61afef        

/* --- Core Applications & Autostart --- */
#define MOJ_TERMINAL         "alacritty"
#define MOJ_LAUNCHER         "dmenu_run"
#define MOJ_SCREENSHOT       "maim ~/screenshot-$(date +%s).png"
#define AUTOSTART_SCRIPT     "bash $HOME/.autoconfigscriptmicwm"
#define POLECENIE_POMOCY     "echo 'Pomoc'"

/* --- Keybindings Configuration --- */
#define KLUCZ_TERMINAL       XK_Return       
#define KLUCZ_LAUNCHER       XK_p            
#define KLUCZ_ZAMKNIJ        XK_q            
#define KLUCZ_LAYOUT         XK_s            
#define KLUCZ_SCREENSHOT     XK_s            
#define KLUCZ_PRINT_SCREEN   XK_Print        
#define KLUCZ_LOCK_FOCUS     XK_d            
#define KLUCZ_FLOATING       XK_space        
#define KLUCZ_FOCUS_NEXT     XK_j            
#define KLUCZ_FOCUS_PREV     XK_k            
#define KLUCZ_MASTER_MINUS   XK_h            
#define KLUCZ_MASTER_PLUS    XK_l            
#define KLUCZ_RESTART        XK_r            
#define KLUCZ_MAKSYMALIZUJ   XK_f            
#define KLUCZ_ROZMIAR_W      XK_w            
#define KLUCZ_GAPS_PLUS      XK_equal        
#define KLUCZ_GAPS_MINUS     XK_minus        
#define KLUCZ_POMOCY         XK_u            

#endif
