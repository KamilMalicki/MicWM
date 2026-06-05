#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/XF86keysym.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <locale.h>
#include "config.h"

typedef struct {
<<<<<<< Updated upstream
    char *name;
    char *prompt;
} Lang;

/* Language: PL, EN, TR, AR */
Lang languages[] = {
    {"PL", " URUCHOM: "},
    {"EN", " RUN: "},
    {"TR", " ÇALIŞTIR: "},
    {"AR", " :تشغيل "} // troche naciągane bo nie jest od prawej do lewej // TODO
};

#define MAX(a, b) ((a) > (b) ? (a) : (b))
=======
    Window win;
    int tag;        // 1-9
    int floating;
    int fullscreen; // 1 = pełny ekran, 0 = normalny
    int valid;      // 1 = active, 0 = pending removal
} Client;

Client clients[MAX_CLIENTS];
int client_count = 0;
int gaps = 10; 

Atom net_wm_window_type, net_wm_window_type_dock, net_current_desktop, net_active_window;
int current_layout = 0;
int active_tag = 1;
double master_fact = 0.55;
>>>>>>> Stashed changes

int xerror(Display *dpy, XErrorEvent *ee) { (void)dpy; (void)ee; return 0; }

<<<<<<< Updated upstream
int cur_lang = 0; // na start jest polski

void init_de_language() {
    if (strcmp(LANGUAGE, "EN") == 0) cur_lang = 1;
    else if (strcmp(LANGUAGE, "TR") == 0) cur_lang = 2;
    else if (strcmp(LANGUAGE, "AR") == 0) cur_lang = 3;
    else cur_lang = 0;  // na start jest polski
}

int main(void) {
    Display *dpy;
    Window root, bar;
    XWindowAttributes attr;
    XButtonEvent start;
    XEvent ev;
    Cursor cursor;
    GC gc;
    char status_text[256];
    Atom wm_protocols, wm_delete_window;
    int in_prompt = 0;
    char prompt_buf[256] = "";
    int prompt_len = 0;
    Window locked_window = None;

    XFontSet fontset;
    char **missing_charsets;
    int missing_count;
    char *def_string;

    if(!(dpy = XOpenDisplay(NULL))) return 1;

    //nowa czcionka
    setlocale(LC_ALL, ""); 

    fontset = XCreateFontSet(dpy, "-*-fixed-medium-r-normal-*-14-*-*-*-*-*-*-*,*",
                            &missing_charsets, &missing_count, &def_string);

    if (missing_count > 0) XFreeStringList(missing_charsets);

    init_de_language();

    strncpy(status_text, TEKST_NA_PASKU, sizeof(status_text) - 1);
    signal(SIGCHLD, SIG_IGN);

    XSetErrorHandler(xerror);

=======
void spawn(const char *cmd) {
    if (fork() == 0) {
        setsid();
        execl("/bin/sh", "sh", "-c", cmd, NULL);
        exit(0);
    }
}

Client *client_find(Window w) {
    for (int i = 0; i < client_count; i++)
        if (clients[i].valid && clients[i].win == w)
            return &clients[i];
    return NULL;
}

void client_add(Window w, int tag, int floating) {
    if (client_find(w)) return;
    if (client_count >= MAX_CLIENTS) return;
    clients[client_count].win = w;
    clients[client_count].tag = tag;
    clients[client_count].floating = floating;
    clients[client_count].fullscreen = 0;
    clients[client_count].valid = 1;
    client_count++;
}

void client_remove(Window w) {
    for (int i = 0; i < client_count; i++) {
        if (clients[i].win == w) {
            clients[i].valid = 0;
            for (int j = i; j < client_count - 1; j++) {
                clients[j] = clients[j + 1];
            }
            client_count--;
            break;
        }
    }
}

int is_dock(Display *dpy, Window w) {
    Atom actual_type; int actual_format; unsigned long nitems, bytes_after;
    unsigned char *prop = NULL;
    int status = XGetWindowProperty(dpy, w, net_wm_window_type, 0, 1, False, XA_ATOM,
                                    &actual_type, &actual_format, &nitems, &bytes_after, &prop);
    if (status == Success && prop) {
        Atom *atoms = (Atom *)prop;
        for (unsigned long i = 0; i < nitems; i++) {
            if (atoms[i] == net_wm_window_type_dock) { XFree(prop); return 1; }
        }
        XFree(prop);
    }
    return 0;
}

void setup_ewmh(Display *dpy, Window root) {
    Atom net_supported = XInternAtom(dpy, "_NET_SUPPORTED", False);
    Atom net_num = XInternAtom(dpy, "_NET_NUMBER_OF_DESKTOPS", False);
    Atom supported[] = { net_num, net_current_desktop, net_active_window };
    XChangeProperty(dpy, root, net_supported, XA_ATOM, 32, PropModeReplace, (unsigned char *)supported, 3);
    long num_desktops = 9;
    XChangeProperty(dpy, root, net_num, XA_CARDINAL, 32, PropModeReplace, (unsigned char *)&num_desktops, 1);
}

void update_polybar_tags(Display *dpy, Window root) {
    long cur_desktop = active_tag - 1;
    XChangeProperty(dpy, root, net_current_desktop, XA_CARDINAL, 32, PropModeReplace, (unsigned char *)&cur_desktop, 1);
}

void set_active_window(Display *dpy, Window root, Window w) {
    if (w == None) XDeleteProperty(dpy, root, net_active_window);
    else XChangeProperty(dpy, root, net_active_window, XA_WINDOW, 32, PropModeReplace, (unsigned char *)&w, 1);
}

void update_borders(Display *dpy, Window focused_win) {
    for (int i = 0; i < client_count; i++) {
        if (!clients[i].valid || clients[i].tag != active_tag) continue;
        if (clients[i].fullscreen) continue;

        if (clients[i].win == focused_win) {
            XSetWindowBorder(dpy, clients[i].win, KOLOR_FOCUS);
        } else {
            XSetWindowBorder(dpy, clients[i].win, KOLOR_ZWYKLY);
        }
    }
}

// =============================================================
// LAYOUT ENGINE (PROPER FULLSCREEN & TILES MIXER)
// =============================================================
void apply_layout(Display *dpy, int screen) {
    int sw = DisplayWidth(dpy, screen);
    int sh = DisplayHeight(dpy, screen);
    int usable_h = sh - MARGINES_DOLNY;

    // 1. Obsłuż okna na pełnym ekranie
    for (int i = 0; i < client_count; i++) {
        if (!clients[i].valid || clients[i].tag != active_tag) continue;
        if (clients[i].fullscreen) {
            XSetWindowBorderWidth(dpy, clients[i].win, 0);
            XMoveResizeWindow(dpy, clients[i].win, 0, 0, sw, sh);
            XRaiseWindow(dpy, clients[i].win);
        }
    }

    // 2. Policz okna kafelkowe
    int tiled_count = 0;
    Window tiled[MAX_CLIENTS];
    for (int i = 0; i < client_count; i++) {
        if (!clients[i].valid || clients[i].tag != active_tag || clients[i].floating || clients[i].fullscreen) continue;
        tiled[tiled_count++] = clients[i].win;
    }

    if (tiled_count == 0) return;

    // 3. Pozycjonuj kafelki
    for (int i = 0; i < tiled_count; i++) {
        int nx = 0, ny = 0, nw = sw, nh = usable_h;

        if (current_layout == 0) { // Master-Stack
            if (tiled_count == 1) {
                nx = 0; ny = 0; nw = sw; nh = usable_h;
            } else if (i == 0) {
                nx = 0; ny = 0; nw = (int)(sw * master_fact); nh = usable_h;
            } else {
                int sc = tiled_count - 1;
                nx = (int)(sw * master_fact); nw = sw - nx;
                nh = usable_h / sc; ny = (i - 1) * nh;
                if (i == tiled_count - 1) nh = usable_h - ny;
            }
        } else if (current_layout == 1) { // Grid
            int rows = 1, cols = 1;
            while (rows * cols < tiled_count) { if (cols == rows) cols++; else rows++; }
            int r = i / cols; int c = i % cols;
            nw = sw / cols; nh = usable_h / rows; nx = c * nw; ny = r * nh;
        }

        nx += gaps; ny += gaps; nw -= 2 * gaps; nh -= 2 * gaps;
        nw -= 2 * GRUBOSC_RAMKI; nh -= 2 * GRUBOSC_RAMKI;
        if (nw < MIN_ROZMIAR) nw = MIN_ROZMIAR;
        if (nh < MIN_ROZMIAR) nh = MIN_ROZMIAR;

        XSetWindowBorderWidth(dpy, tiled[i], GRUBOSC_RAMKI);
        XMoveResizeWindow(dpy, tiled[i], nx, ny, nw, nh);
    }
}

void focus_fallback(Display *dpy, Window root) {
    Window first_valid = None;
    for (int i = 0; i < client_count; i++) {
        if (clients[i].valid && clients[i].tag == active_tag) {
            first_valid = clients[i].win;
            break;
        }
    }
    if (first_valid != None) {
        XSetInputFocus(dpy, first_valid, RevertToParent, CurrentTime);
        set_active_window(dpy, root, first_valid);
        update_borders(dpy, first_valid);
    } else {
        XSetInputFocus(dpy, root, RevertToParent, CurrentTime);
        set_active_window(dpy, root, None);
    }
}

// =============================================================
// INTERACTIVE ACTIONS & WORKSPACE SWITCHING
// =============================================================
void view_tag(Display *dpy, int screen, int tag) {
    if (tag < 1 || tag > 9) return;
    active_tag = tag;
    
    Window root = DefaultRootWindow(dpy);
    update_polybar_tags(dpy, root);

    int sw = DisplayWidth(dpy, screen);
    int sh = DisplayHeight(dpy, screen);

    for (int i = 0; i < client_count; i++) {
        if (!clients[i].valid) continue;
        if (clients[i].tag == active_tag) {
            XMapWindow(dpy, clients[i].win);
        } else {
            // Bezpieczne ukrywanie okien bez zrywania focusu systemowego
            XMoveWindow(dpy, clients[i].win, sw * 2, sh * 2);
        }
    }

    apply_layout(dpy, screen);
    focus_fallback(dpy, root);
    XFlush(dpy);
}

void tag_window(Display *dpy, int screen, Window w, int tag) {
    Client *c = client_find(w);
    if (!c || tag < 1 || tag > 9) return;
    c->tag = tag;

    int sw = DisplayWidth(dpy, screen);
    int sh = DisplayHeight(dpy, screen);
    if (tag != active_tag) XMoveWindow(dpy, w, sw * 2, sh * 2);

    apply_layout(dpy, screen);
    focus_fallback(dpy, DefaultRootWindow(dpy));
}

void toggle_fullscreen(Display *dpy, int screen) {
    Window focused; int revert_to;
    XGetInputFocus(dpy, &focused, &revert_to);
    
    // Jeśli focus jest pusty, szukamy pierwszego okna na obecnym tagu
    if (focused == None || focused == DefaultRootWindow(dpy)) {
        for (int i = 0; i < client_count; i++) {
            if (clients[i].valid && clients[i].tag == active_tag) {
                focused = clients[i].win;
                break;
            }
        }
    }

    Client *c = client_find(focused);
    if (!c) return;

    c->fullscreen = !c->fullscreen;
    
    if (!c->fullscreen) {
        XSetWindowBorderWidth(dpy, focused, GRUBOSC_RAMKI);
    }
    
    apply_layout(dpy, screen);
    XSetInputFocus(dpy, focused, RevertToParent, CurrentTime);
    set_active_window(dpy, DefaultRootWindow(dpy), focused);
    update_borders(dpy, focused);
}

void focus_stack(Display *dpy, int dir) {
    Window wins[MAX_CLIENTS]; int cnt = 0; int cur = -1;
    Window focused; int revert_to;
    XGetInputFocus(dpy, &focused, &revert_to);

    for (int i = 0; i < client_count; i++) {
        if (!clients[i].valid || clients[i].tag != active_tag) continue;
        wins[cnt] = clients[i].win;
        if (clients[i].win == focused) cur = cnt;
        cnt++;
    }
    if (cnt <= 1) return;

    int next = 0;
    if (cur != -1) next = (cur + dir + cnt) % cnt;

    XSetInputFocus(dpy, wins[next], RevertToParent, CurrentTime);
    update_borders(dpy, wins[next]);
    XRaiseWindow(dpy, wins[next]);
    set_active_window(dpy, DefaultRootWindow(dpy), wins[next]);
}

// =============================================================
// MAIN INITIALIZATION & EVENT LOOP
// =============================================================
int main(void) {
    Display *dpy = XOpenDisplay(NULL);
    if (!dpy) return 1;

>>>>>>> Stashed changes
    int screen = DefaultScreen(dpy);
    Window root = DefaultRootWindow(dpy);
    XWindowAttributes attr; XButtonEvent start; XEvent ev;

<<<<<<< Updated upstream
    wm_protocols = XInternAtom(dpy, "WM_PROTOCOLS", False);
    wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", False);

    cursor = XCreateFontCursor(dpy, XC_left_ptr);
    XDefineCursor(dpy, root, cursor);

    XSetWindowAttributes bar_attr;
    bar_attr.override_redirect = True;
    bar_attr.background_pixel = BlackPixel(dpy, screen);

    bar = XCreateWindow(dpy, root, 0, 0, DisplayWidth(dpy, screen), WYSOKOSC_PASKA, 0,
                        CopyFromParent, InputOutput, CopyFromParent,
                        CWOverrideRedirect | CWBackPixel, &bar_attr);

    gc = XCreateGC(dpy, bar, 0, NULL);
    XSetForeground(dpy, gc, WhitePixel(dpy, screen));
    
    XSelectInput(dpy, bar, ExposureMask);
    XMapWindow(dpy, bar);

    XSelectInput(dpy, root, SubstructureRedirectMask | SubstructureNotifyMask | PropertyChangeMask);

    XGrabKey(dpy, XKeysymToKeycode(dpy, XK_t), MOD_KEY, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XK_p), MOD_KEY, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XK_q), MOD_KEY, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XK_s), MOD_KEY, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XK_d), MOD_KEY, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XK_q), MOD_KEY | ShiftMask, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XK_f), MOD_KEY | ShiftMask, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XK_w), MOD_KEY | ShiftMask, root, True, GrabModeAsync, GrabModeAsync);

    XGrabKey(dpy, XKeysymToKeycode(dpy, XF86XK_AudioRaiseVolume), 0, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XF86XK_AudioLowerVolume), 0, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XF86XK_AudioMute), 0, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XF86XK_MonBrightnessUp), 0, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XF86XK_MonBrightnessDown), 0, root, True, GrabModeAsync, GrabModeAsync);

    XGrabButton(dpy, 1, MOD_KEY, root, True, ButtonPressMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(dpy, 3, MOD_KEY, root, True, ButtonPressMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);

    start.subwindow = None;
    start.button = 0;
=======
    XSetErrorHandler(xerror);
    net_wm_window_type = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
    net_wm_window_type_dock = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);
    net_current_desktop = XInternAtom(dpy, "_NET_CURRENT_DESKTOP", False);
    net_active_window = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);

    setup_ewmh(dpy, root);
    XDefineCursor(dpy, root, XCreateFontCursor(dpy, XC_left_ptr));
    
    // SubstructureRedirectMask jest krytyczny, by przechwycić kliknięcia w Polybara
    XSelectInput(dpy, root, SubstructureRedirectMask | SubstructureNotifyMask | PropertyChangeMask | EnterWindowMask);
    update_polybar_tags(dpy, root);

    // Rejestracja klawiszy
    XGrabKey(dpy, XKeysymToKeycode(dpy, KLUCZ_TERMINAL), MOD_KEY, root, False, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, KLUCZ_LAUNCHER), MOD_KEY, root, False, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, KLUCZ_ZAMKNIJ), MOD_KEY, root, False, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, KLUCZ_LAYOUT), MOD_KEY, root, False, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, KLUCZ_FOCUS_NEXT), MOD_KEY, root, False, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, KLUCZ_FOCUS_PREV), MOD_KEY, root, False, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, KLUCZ_GAPS_PLUS), MOD_KEY, root, False, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, KLUCZ_GAPS_MINUS), MOD_KEY, root, False, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, KLUCZ_FLOATING), MOD_KEY, root, False, GrabModeAsync, GrabModeAsync);
    
    XGrabKey(dpy, XKeysymToKeycode(dpy, KLUCZ_MAKSYMALIZUJ), MOD_KEY | ShiftMask, root, False, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, KLUCZ_MASTER_MINUS), MOD_KEY | ShiftMask, root, False, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, KLUCZ_MASTER_PLUS), MOD_KEY | ShiftMask, root, False, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, KLUCZ_RESTART), MOD_KEY | ShiftMask, root, False, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, KLUCZ_ROZMIAR_W), MOD_KEY | ShiftMask, root, False, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, KLUCZ_POMOCY), MOD_KEY | ShiftMask, root, False, GrabModeAsync, GrabModeAsync);

    for (int i = 0; i < 9; i++) {
        XGrabKey(dpy, XKeysymToKeycode(dpy, XK_1 + i), MOD_KEY, root, False, GrabModeAsync, GrabModeAsync);
        XGrabKey(dpy, XKeysymToKeycode(dpy, XK_1 + i), MOD_KEY | ShiftMask, root, False, GrabModeAsync, GrabModeAsync);
    }

    XGrabButton(dpy, 1, MOD_KEY, root, True, ButtonPressMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(dpy, 3, MOD_KEY, root, True, ButtonPressMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);

    start.subwindow = None;
    spawn(AUTOSTART_SCRIPT);
>>>>>>> Stashed changes

    if(system(AUTOSTART_SCRIPT)) {}

    while(1) {
        XNextEvent(dpy, &ev);
        XRaiseWindow(dpy, bar);

<<<<<<< Updated upstream
        if(ev.type == Expose && ev.xexpose.window == bar) {
            XClearWindow(dpy, bar);
            if(in_prompt) {
                char temp[300];
                snprintf(temp, sizeof(temp), " %s: %s_", languages[cur_lang].prompt ,prompt_buf);
                //XDrawString(dpy, bar, gc, 10, 14, temp, strlen(temp)); stare wyświetlanie
                Xutf8DrawString(dpy, bar, fontset, gc, 10, 14, temp, strlen(temp));
            } else {
                Xutf8DrawString(dpy, bar, fontset, gc, 10, 14, status_text, strlen(status_text));
                //XDrawString(dpy, bar, gc, 10, 14, status_text, strlen(status_text)); stare wyświetlanie
            }
        }
        else if(ev.type == PropertyNotify && ev.xproperty.window == root && ev.xproperty.atom == XA_WM_NAME) {
            char *name = NULL;
            if(XFetchName(dpy, root, &name) && name) {
                strncpy(status_text, name, sizeof(status_text) - 1);
                status_text[sizeof(status_text) - 1] = '\0';
                XFree(name);
            }
            if(!in_prompt) {
                XClearWindow(dpy, bar);
                //XDrawString(dpy, bar, gc, 10, 14, status_text, strlen(status_text));
                Xutf8DrawString(dpy, bar, fontset, gc, 10, 14, status_text, strlen(status_text));
            }
        }
        else if(ev.type == MapRequest) {
            if (ev.xmaprequest.window == bar) continue;

            XWindowAttributes wa, cwa;
            XGetWindowAttributes(dpy, root, &wa);
            XGetWindowAttributes(dpy, ev.xmaprequest.window, &cwa);

            int x = (wa.width - cwa.width) / 2;
            int y = (wa.height - cwa.height) / 2;
            if (y < WYSOKOSC_PASKA) y = WYSOKOSC_PASKA;

            XMoveWindow(dpy, ev.xmaprequest.window, x, y);
            XMapWindow(dpy, ev.xmaprequest.window);
            XSetWindowBorderWidth(dpy, ev.xmaprequest.window, GRUBOSC_RAMKI);
            XSetWindowBorder(dpy, ev.xmaprequest.window, KOLOR_FOCUS);
            XSetInputFocus(dpy, ev.xmaprequest.window, RevertToParent, CurrentTime);
            
            XSelectInput(dpy, ev.xmaprequest.window, EnterWindowMask | LeaveWindowMask);
        }
        else if(ev.type == ConfigureRequest) {
            XWindowChanges wc;
            wc.x = ev.xconfigurerequest.x;
            wc.y = ev.xconfigurerequest.y;
            wc.width = ev.xconfigurerequest.width;
            wc.height = ev.xconfigurerequest.height;
            wc.border_width = ev.xconfigurerequest.border_width;
            wc.sibling = ev.xconfigurerequest.above;
            wc.stack_mode = ev.xconfigurerequest.detail;
            XConfigureWindow(dpy, ev.xconfigurerequest.window, ev.xconfigurerequest.value_mask, &wc);
        }
        else if(ev.type == EnterNotify && ev.xcrossing.window != bar && ev.xcrossing.window != root) {
            if(ev.xcrossing.window != locked_window) {
                XSetInputFocus(dpy, ev.xcrossing.window, RevertToParent, CurrentTime);
                XSetWindowBorder(dpy, ev.xcrossing.window, KOLOR_FOCUS);
            }
        }
        else if(ev.type == LeaveNotify && ev.xcrossing.window != bar && ev.xcrossing.window != root) {
            if(ev.xcrossing.window != locked_window) {
                XSetWindowBorder(dpy, ev.xcrossing.window, KOLOR_ZWYKLY);
            }
        }
        else if(ev.type == KeyPress) {
            KeySym ks;
            char keybuf[32];
            int klen = XLookupString(&ev.xkey, keybuf, sizeof(keybuf), &ks, NULL);
            KeySym base_ks = XLookupKeysym(&ev.xkey, 0);

            if(in_prompt) {
                if(ks == XK_Return) {
                    if(prompt_len > 0) {
                        if(fork() == 0) {
                            close(ConnectionNumber(dpy));
                            execlp("/bin/sh", "sh", "-c", prompt_buf, NULL);
                            _exit(0);
                        }
                    }
                    in_prompt = 0;
                    prompt_len = 0;
                    prompt_buf[0] = '\0';
                    XUngrabKeyboard(dpy, CurrentTime);
                    XClearWindow(dpy, bar);
                    //XDrawString(dpy, bar, gc, 10, 14, status_text, strlen(status_text));
                    Xutf8DrawString(dpy, bar, fontset, gc, 10, 14, status_text, strlen(status_text));
                } else if(ks == XK_Escape) {
                    in_prompt = 0;
                    prompt_len = 0;
                    prompt_buf[0] = '\0';
                    XUngrabKeyboard(dpy, CurrentTime);
                    XClearWindow(dpy, bar);
                    //XDrawString(dpy, bar, gc, 10, 14, status_text, strlen(status_text));
                    Xutf8DrawString(dpy, bar, fontset, gc, 10, 14, status_text, strlen(status_text));
                } else if(ks == XK_BackSpace) {
                    if(prompt_len > 0) {
                        prompt_len--;
                        prompt_buf[prompt_len] = '\0';
                    }
                    XClearWindow(dpy, bar);
                    char temp[300];
                    snprintf(temp, sizeof(temp), " %s%s_", languages[cur_lang].prompt ,prompt_buf);

                    Xutf8DrawString(dpy, bar, fontset, gc, 10, 14, temp, strlen(temp));
                        //XDrawString(dpy, bar, gc, 10, 14, temp, strlen(temp)); stare wyświetlanie
                } else if(klen > 0 && prompt_len + klen < sizeof(prompt_buf) - 1 && keybuf[0] >= 32 && keybuf[0] <= 126) {
                    strncpy(prompt_buf + prompt_len, keybuf, klen);
                    prompt_len += klen;
                    prompt_buf[prompt_len] = '\0';
                    XClearWindow(dpy, bar);
                    char temp[300];
                    snprintf(temp, sizeof(temp), " %s%s_",languages[cur_lang].prompt ,prompt_buf);
                    // XDrawString(dpy, bar, gc, 10, 14, temp, strlen(temp)); stare wyświetlanie
                    Xutf8DrawString(dpy, bar, fontset, gc, 10, 14, temp, strlen(temp));
                }
                continue;
            }

            if(base_ks == XK_p && (ev.xkey.state & MOD_KEY)) {
                in_prompt = 1;
                prompt_len = 0;
                prompt_buf[0] = '\0';
                XGrabKeyboard(dpy, root, True, GrabModeAsync, GrabModeAsync, CurrentTime);
                XClearWindow(dpy, bar);
                char buf[128];
                snprintf(buf, sizeof(buf), " %s_", languages[cur_lang].prompt);
                //XDrawString(dpy, bar, gc, 10, 14, buf, strlen(buf));
                Xutf8DrawString(dpy, bar, fontset, gc, 10, 14, buf, strlen(buf));
            }
            else if(base_ks == XK_d && (ev.xkey.state & MOD_KEY)) {
                if (locked_window == None) {
                    Window focused;
                    int revert_to;
                    XGetInputFocus(dpy, &focused, &revert_to);
                    if (focused != root && focused != bar && focused != None) {
                        locked_window = focused;
                        XSetInputFocus(dpy, root, RevertToParent, CurrentTime);
                        XSetWindowBorder(dpy, locked_window, KOLOR_ZWYKLY);
                    }
                } else {
                    locked_window = None;
                }
            }
            else if(base_ks == XK_s && (ev.xkey.state & MOD_KEY)) {
                Window d1, d2, *wins = NULL;
                unsigned int num = 0;
                if(XQueryTree(dpy, root, &d1, &d2, &wins, &num)) {
                    int valid_count = 0;
                    Window valid_wins[100];
                    for(unsigned int i = 0; i < num; i++) {
                        XWindowAttributes wa;
                        if(wins[i] != bar && XGetWindowAttributes(dpy, wins[i], &wa) && wa.map_state == IsViewable && !wa.override_redirect) {
                            if (valid_count < 100) valid_wins[valid_count++] = wins[i];
                        }
                    }
                    if(wins) XFree(wins);

                    int sw = DisplayWidth(dpy, screen);
                    int sh = DisplayHeight(dpy, screen) - WYSOKOSC_PASKA;
                    int sy = WYSOKOSC_PASKA;

                    for (int i = 0; i < valid_count; i++) {
                        int nx, ny, nw, nh;
                        if (valid_count == 1) {
                            nx = 0; ny = sy; nw = sw; nh = sh;
                        } else if (i == 0) {
                            nx = 0; ny = sy; nw = sw / 2; nh = sh;
                        } else {
                            int sc = valid_count - 1;
                            nx = sw / 2;
                            nw = sw - (sw / 2);
                            nh = sh / sc;
                            ny = sy + (i - 1) * nh;
                            if (i == valid_count - 1) nh = sh - (ny - sy);
                        }
                        nw -= 2 * GRUBOSC_RAMKI;
                        nh -= 2 * GRUBOSC_RAMKI;
                        if(nw < MIN_ROZMIAR) nw = MIN_ROZMIAR;
                        if(nh < MIN_ROZMIAR) nh = MIN_ROZMIAR;
                        
                        XMoveResizeWindow(dpy, valid_wins[i], nx, ny, nw, nh);
                        XSetWindowBorderWidth(dpy, valid_wins[i], GRUBOSC_RAMKI);
                        XRaiseWindow(dpy, valid_wins[i]);
                    }
                }
            }
            else if(base_ks == XK_q && (ev.xkey.state & ShiftMask)) break;
            else if(base_ks == XF86XK_AudioRaiseVolume) system("v=$(amixer sset Master 5%+ | grep -m1 -oE '[0-9]+%') && xsetroot -name \" [ VOLUME: $v ] \" &");
            else if(base_ks == XF86XK_AudioLowerVolume) system("v=$(amixer sset Master 5%- | grep -m1 -oE '[0-9]+%') && xsetroot -name \" [ VOLUME: $v ] \" &");
            else if(base_ks == XF86XK_AudioMute) system("xsetroot -name \" [ AUDIO MUTED / UNMUTED ] \" && amixer sset Master toggle &");
            else if(base_ks == XF86XK_MonBrightnessUp) system("b=$(brightnessctl set +5% | grep -m1 -oE '[0-9]+%') && xsetroot -name \" [ BRIGHTNESS: $b ] \" &");
            else if(base_ks == XF86XK_MonBrightnessDown) system("b=$(brightnessctl set 5%- | grep -m1 -oE '[0-9]+%') && xsetroot -name \" [ BRIGHTNESS: $b ] \" &");
            else if(base_ks == XK_q && ev.xkey.subwindow != None && ev.xkey.subwindow != bar) {
                XEvent ke;
                ke.type = ClientMessage;
                ke.xclient.window = ev.xkey.subwindow;
                ke.xclient.message_type = wm_protocols;
                ke.xclient.format = 32;
                ke.xclient.data.l[0] = wm_delete_window;
                ke.xclient.data.l[1] = CurrentTime;
                XSendEvent(dpy, ev.xkey.subwindow, False, NoEventMask, &ke);
            }
            else if(base_ks == XK_f && (ev.xkey.state & ShiftMask) && ev.xkey.subwindow != None && ev.xkey.subwindow != bar) {
                XMoveResizeWindow(dpy, ev.xkey.subwindow, 0, WYSOKOSC_PASKA, 
                                 DisplayWidth(dpy, screen), DisplayHeight(dpy, screen) - WYSOKOSC_PASKA);
                XSetWindowBorderWidth(dpy, ev.xkey.subwindow, 0);
                XRaiseWindow(dpy, ev.xkey.subwindow);
            }
            else if(base_ks == XK_w && (ev.xkey.state & ShiftMask) && ev.xkey.subwindow != None && ev.xkey.subwindow != bar) {
                XMoveResizeWindow(dpy, ev.xkey.subwindow, (DisplayWidth(dpy, screen)-800)/2, (DisplayHeight(dpy, screen)-600)/2, 800, 600);
                XSetWindowBorderWidth(dpy, ev.xkey.subwindow, GRUBOSC_RAMKI);
                XSetWindowBorder(dpy, ev.xkey.subwindow, KOLOR_FOCUS);
            }
            else if(base_ks == XK_t) {
                if(fork() == 0) {
                    close(ConnectionNumber(dpy));
                    execlp(MOJ_TERMINAL, MOJ_TERMINAL, NULL);
                    _exit(0);
                }
            }
        }
        else if(ev.type == ButtonPress && ev.xbutton.subwindow != None && ev.xbutton.subwindow != bar) {
            if(ev.xbutton.subwindow != locked_window) {
                XGetWindowAttributes(dpy, ev.xbutton.subwindow, &attr);
                XSetInputFocus(dpy, ev.xbutton.subwindow, RevertToParent, CurrentTime);
                XRaiseWindow(dpy, ev.xbutton.subwindow);
                
                if (attr.border_width > 0) { 
                    start = ev.xbutton; 
                    XSetWindowBorder(dpy, ev.xbutton.subwindow, KOLOR_FOCUS); 
                } else { 
                    start.subwindow = None; 
                }
            }
=======
        if (ev.type == MapRequest) {
            Window w = ev.xmaprequest.window;
            if (is_dock(dpy, w)) { XMapRaised(dpy, w); continue; }

            client_add(w, active_tag, 0);
            XSetWindowBorderWidth(dpy, w, GRUBOSC_RAMKI);
            XMapWindow(dpy, w);
            XSelectInput(dpy, w, EnterWindowMask);
            
            XSetInputFocus(dpy, w, RevertToParent, CurrentTime);
            apply_layout(dpy, screen);
            update_borders(dpy, w);
            set_active_window(dpy, root, w);
        }
        else if (ev.type == DestroyNotify || ev.type == UnmapNotify) {
            Window w = (ev.type == DestroyNotify) ? ev.xdestroywindow.window : ev.xunmap.window;
            if (client_find(w)) {
                client_remove(w);
                apply_layout(dpy, screen);
                focus_fallback(dpy, root);
            }
        }
        else if (ev.type == ClientMessage) {
            // KLUCZOWE: Polybar wysyła to zdarzenie przy kliknięciu myszą w tag.
            // Teraz poprawnie wywołujemy pełną funkcję przekierowania view_tag()!
            if (ev.xclient.message_type == net_current_desktop) {
                int target_tag = ev.xclient.data.l[0] + 1;
                view_tag(dpy, screen, target_tag);
            }
        }
        else if (ev.type == EnterNotify) {
            if (ev.xcrossing.window != root && !is_dock(dpy, ev.xcrossing.window)) {
                Client *c = client_find(ev.xcrossing.window);
                if (c && c->tag == active_tag) {
                    XSetInputFocus(dpy, ev.xcrossing.window, RevertToParent, CurrentTime);
                    update_borders(dpy, ev.xcrossing.window);
                    set_active_window(dpy, root, ev.xcrossing.window);
                }
            }
        }
        else if (ev.type == ButtonPress && ev.xbutton.subwindow != None) {
            if (is_dock(dpy, ev.xbutton.subwindow)) continue;
            XGetWindowAttributes(dpy, ev.xbutton.subwindow, &attr);
            XSetInputFocus(dpy, ev.xbutton.subwindow, RevertToParent, CurrentTime);
            set_active_window(dpy, root, ev.xbutton.subwindow);
            XRaiseWindow(dpy, ev.xbutton.subwindow);
            start = ev.xbutton;
            update_borders(dpy, ev.xbutton.subwindow);
>>>>>>> Stashed changes
        }
        else if(ev.type == MotionNotify && start.subwindow != None) {
            int xdiff = ev.xbutton.x_root - start.x_root;
            int ydiff = ev.xbutton.y_root - start.y_root;
<<<<<<< Updated upstream
            
            int nx = attr.x + (start.button == 1 ? xdiff : 0);
            int ny = attr.y + (start.button == 1 ? ydiff : 0);
            int nw = MAX(MIN_ROZMIAR, attr.width + (start.button == 3 ? xdiff : 0));
            int nh = MAX(MIN_ROZMIAR, attr.height + (start.button == 3 ? ydiff : 0));
            
            if (nx < 0) nx = 0;
            if (ny < WYSOKOSC_PASKA) ny = WYSOKOSC_PASKA;
            if (nx + nw + 2 * GRUBOSC_RAMKI > DisplayWidth(dpy, screen)) nx = DisplayWidth(dpy, screen) - nw - 2 * GRUBOSC_RAMKI;
            if (ny + nh + 2 * GRUBOSC_RAMKI > DisplayHeight(dpy, screen)) ny = DisplayHeight(dpy, screen) - nh - 2 * GRUBOSC_RAMKI;

            XMoveResizeWindow(dpy, start.subwindow, nx, ny, nw, nh);
        }
        else if(ev.type == ButtonRelease && start.subwindow != None) {
            XSetWindowBorder(dpy, start.subwindow, KOLOR_ZWYKLY);
=======
            Client *c = client_find(start.subwindow);
            if (c) c->floating = 1;

            if (start.button == 1) XMoveWindow(dpy, start.subwindow, attr.x + xdiff, attr.y + ydiff);
            else if (start.button == 3) XResizeWindow(dpy, start.subwindow, MAX(MIN_ROZMIAR, attr.width + xdiff), MAX(MIN_ROZMIAR, attr.height + ydiff));
        }
        else if (ev.type == ButtonRelease) {
>>>>>>> Stashed changes
            start.subwindow = None;
        }
        else if (ev.type == KeyPress) {
            KeySym keysym = XLookupKeysym(&ev.xkey, 0);
            int state = ev.xkey.state;

<<<<<<< Updated upstream
    XFreeGC(dpy, gc);
    XFreeCursor(dpy, cursor);
=======
            if (keysym >= XK_1 && keysym <= XK_9) {
                int tag = keysym - XK_1 + 1;
                if (state & ShiftMask) {
                    Window focused; int rev; XGetInputFocus(dpy, &focused, &rev);
                    tag_window(dpy, screen, focused, tag);
                } else {
                    view_tag(dpy, screen, tag);
                }
            }
            else if (keysym == KLUCZ_TERMINAL) spawn(MOJ_TERMINAL);
            else if (keysym == KLUCZ_LAUNCHER) spawn(MOJ_LAUNCHER);
            else if (keysym == KLUCZ_LAYOUT) { current_layout = !current_layout; apply_layout(dpy, screen); }
            else if (keysym == KLUCZ_FOCUS_NEXT) focus_stack(dpy, 1);
            else if (keysym == KLUCZ_FOCUS_PREV) focus_stack(dpy, -1);
            else if (keysym == KLUCZ_MAKSYMALIZUJ) toggle_fullscreen(dpy, screen);
            else if (keysym == KLUCZ_GAPS_PLUS) { gaps += 2; apply_layout(dpy, screen); }
            else if (keysym == KLUCZ_GAPS_MINUS) { gaps -= 2; if (gaps < 0) gaps = 0; apply_layout(dpy, screen); }
            else if (keysym == KLUCZ_ZAMKNIJ) {
                Window focused; int rev; XGetInputFocus(dpy, &focused, &rev);
                if (focused != root && focused != None && !is_dock(dpy, focused)) XKillClient(dpy, focused);
            }
            else if (keysym == KLUCZ_FLOATING) {
                Window focused; int rev; XGetInputFocus(dpy, &focused, &rev);
                Client *c = client_find(focused);
                if (c) { c->floating = !c->floating; apply_layout(dpy, screen); update_borders(dpy, focused); }
            }
            else if (keysym == KLUCZ_RESTART) {
                XCloseDisplay(dpy);
                char *args[] = { "/usr/local/bin/micwm", NULL };
                execv(args[0], args);
                return 0;
            }
        }
    }
>>>>>>> Stashed changes
    XCloseDisplay(dpy);
    return 0;
}