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
#include "config.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MAX_CLIENTS 256

// =============================================================
// CLIENT STRUCT & MANAGEMENT
// =============================================================
typedef struct {
    Window win;
    int tag;        // 1-9
    int floating;
    int valid;      // 1 = active, 0 = pending removal
} Client;

Client clients[MAX_CLIENTS];
int client_count = 0;

int gaps = 10; 

Atom net_wm_window_type, net_wm_window_type_dock, net_current_desktop, net_active_window;
int current_layout = 0;
int active_tag = 1;
double master_fact = 0.55;

int xerror(Display *dpy, XErrorEvent *ee) { return 0; }

void spawn(const char *cmd) {
    if (fork() == 0) {
        setsid();
        execl("/bin/sh", "sh", "-c", cmd, NULL);
        perror("micwm: spawn failed");
        exit(EXIT_FAILURE);
    }
}

Client *client_find(Window w) {
    for (int i = 0; i < client_count; i++)
        if (clients[i].valid && clients[i].win == w)
            return &clients[i];
    return NULL;
}

Client *client_add(Window w, int tag, int floating) {
    Client *existing = client_find(w);
    if (existing) return existing;
    if (client_count >= MAX_CLIENTS) return NULL;
    clients[client_count].win = w;
    clients[client_count].tag = tag;
    clients[client_count].floating = floating;
    clients[client_count].valid = 1;
    return &clients[client_count++];
}

void client_remove(Window w) {
    for (int i = 0; i < client_count; i++)
        if (clients[i].win == w)
            clients[i].valid = 0;
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

int is_help_window(Display *dpy, Window w) {
    XTextProperty prop;
    if (XGetWMName(dpy, w, &prop) && prop.value) {
        if (strcmp((const char *)prop.value, "MicWM_Help") == 0) {
            XFree(prop.value);
            return 1;
        }
        XFree(prop.value);
    }
    return 0;
}

void setup_ewmh(Display *dpy, Window root) {
    Atom net_supported = XInternAtom(dpy, "_NET_SUPPORTED", False);
    Atom net_num = XInternAtom(dpy, "_NET_NUMBER_OF_DESKTOPS", False);
    Atom net_wm_desktop = XInternAtom(dpy, "_NET_WM_DESKTOP", False);
    Atom net_names = XInternAtom(dpy, "_NET_DESKTOP_NAMES", False);
    net_current_desktop = XInternAtom(dpy, "_NET_CURRENT_DESKTOP", False);
    net_active_window = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);

    Atom supported[] = { net_num, net_current_desktop, net_wm_desktop, net_names, net_active_window };
    XChangeProperty(dpy, root, net_supported, XA_ATOM, 32, PropModeReplace, (unsigned char *)supported, 5);

    long num_desktops = 9;
    XChangeProperty(dpy, root, net_num, XA_CARDINAL, 32, PropModeReplace, (unsigned char *)&num_desktops, 1);

    char names[] = "1\0" "2\0" "3\0" "4\0" "5\0" "6\0" "7\0" "8\0" "9\0";
    XChangeProperty(dpy, root, net_names, XInternAtom(dpy, "UTF8_STRING", False), 8, PropModeReplace, (unsigned char *)names, 18);
}

void set_net_wm_desktop(Display *dpy, Window w, int tag) {
    Atom net_wm_desktop = XInternAtom(dpy, "_NET_WM_DESKTOP", False);
    long ewmh_desktop = tag - 1;
    XChangeProperty(dpy, w, net_wm_desktop, XA_CARDINAL, 32, PropModeReplace, (unsigned char *)&ewmh_desktop, 1);
}

void update_polybar_tags(Display *dpy, Window root) {
    long cur_desktop = active_tag - 1;
    XChangeProperty(dpy, root, net_current_desktop, XA_CARDINAL, 32, PropModeReplace, (unsigned char *)&cur_desktop, 1);
}

void set_active_window(Display *dpy, Window root, Window w) {
    if (w == None) {
        XDeleteProperty(dpy, root, net_active_window);
    } else {
        XChangeProperty(dpy, root, net_active_window, XA_WINDOW, 32, PropModeReplace, (unsigned char *)&w, 1);
    }
}

int is_fullscreen_or_util(Display *dpy, Window w, int screen) {
    XWindowAttributes wa;
    if (XGetWindowAttributes(dpy, w, &wa)) {
        if (wa.width >= DisplayWidth(dpy, screen) && wa.height >= (DisplayHeight(dpy, screen) - MARGINES_DOLNY)) return 1;
    }


    XClassHint ch;
    if (XGetClassHint(dpy, w, &ch)) {
        if (ch.res_name && strcmp(ch.res_name, "feh") == 0) {
            XFree(ch.res_name); if (ch.res_class) XFree(ch.res_class);
            return 1; 
        }
        if (ch.res_name) XFree(ch.res_name);
        if (ch.res_class) XFree(ch.res_class);
    }


    Atom actual_type; int actual_format; unsigned long nitems, bytes_after;
    unsigned char *prop = NULL;
    Atom wtype = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);

    if (XGetWindowProperty(dpy, w, wtype, 0, 1, False, XA_ATOM,
                           &actual_type, &actual_format, &nitems, &bytes_after, &prop) == Success && prop) {
        Atom *atoms = (Atom *)prop;
        for (unsigned long i = 0; i < nitems; i++) {
            if (atoms[i] == XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", False) ||
                atoms[i] == XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_SPLASH", False) ||
                atoms[i] == XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_UTILITY", False)) {
                XFree(prop); return 1;
            }
        }
        XFree(prop);
    }
    Window trans;
    if (XGetTransientForHint(dpy, w, &trans)) return 1;
    return 0;
}

// =============================================================
// LAYOUT & FOCUS MANAGMENT
// =============================================================
void apply_layout(Display *dpy, int screen) {
    Window tiled[MAX_CLIENTS];
    int tiled_count = 0;

    for (int i = 0; i < client_count; i++) {
        if (!clients[i].valid) continue;
        if (clients[i].tag != active_tag) continue;
        if (clients[i].floating) continue;
        if (tiled_count < MAX_CLIENTS)
            tiled[tiled_count++] = clients[i].win;
    }

    if (tiled_count == 0) return;

    int sw = DisplayWidth(dpy, screen);
    int sh = DisplayHeight(dpy, screen);
    int usable_h = sh - MARGINES_DOLNY;

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

        XMoveResizeWindow(dpy, tiled[i], nx, ny, nw, nh);
        XSetWindowBorderWidth(dpy, tiled[i], GRUBOSC_RAMKI);
        XRaiseWindow(dpy, tiled[i]);
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
        XSetWindowBorder(dpy, first_valid, KOLOR_FOCUS);
    } else {
        XSetInputFocus(dpy, root, RevertToParent, CurrentTime);
        set_active_window(dpy, root, None);
    }
}

// =============================================================
// WORKSPACE SWITCH
// =============================================================
void view_tag(Display *dpy, int screen, int tag) {
    if (tag == active_tag) return;
    active_tag = tag;
    update_polybar_tags(dpy, DefaultRootWindow(dpy));

    int sw = DisplayWidth(dpy, screen);
    int sh = DisplayHeight(dpy, screen);

    for (int i = 0; i < client_count; i++) {
        if (!clients[i].valid) continue;
        if (clients[i].tag == active_tag) {
            XMapWindow(dpy, clients[i].win);
        } else {
            XMoveWindow(dpy, clients[i].win, sw * 2, sh * 2);
        }
    }

    apply_layout(dpy, screen);
    focus_fallback(dpy, DefaultRootWindow(dpy));
    XFlush(dpy);
}

void tag_window(Display *dpy, int screen, Window w, int tag) {
    Client *c = client_find(w);
    if (!c) return;
    c->tag = tag;
    set_net_wm_desktop(dpy, w, tag);

    int sw = DisplayWidth(dpy, screen);
    int sh = DisplayHeight(dpy, screen);
    if (tag != active_tag)
        XMoveWindow(dpy, w, sw * 2, sh * 2);

    apply_layout(dpy, screen);
    focus_fallback(dpy, DefaultRootWindow(dpy));
}

void toggle_floating(Display *dpy, int screen) {
    Window focused; int revert_to;
    XGetInputFocus(dpy, &focused, &revert_to);
    Client *c = client_find(focused);
    if (!c) return;

    c->floating = !c->floating;
    if (c->floating) {
        int sw = DisplayWidth(dpy, screen);
        int sh = DisplayHeight(dpy, screen) - MARGINES_DOLNY;
        XMoveResizeWindow(dpy, focused, (sw - 800) / 2, (sh - 600) / 2, 800, 600);
    }
    XSetWindowBorderWidth(dpy, focused, GRUBOSC_RAMKI);
    apply_layout(dpy, screen);
}

void focus_stack(Display *dpy, int dir) {
    Window wins[MAX_CLIENTS]; int cnt = 0; int cur = -1;
    Window focused; int revert_to;
    XGetInputFocus(dpy, &focused, &revert_to);

    for (int i = 0; i < client_count; i++) {
        if (!clients[i].valid) continue;
        if (clients[i].tag != active_tag) continue;
        wins[cnt] = clients[i].win;
        if (clients[i].win == focused) cur = cnt;
        cnt++;
    }
    if (cnt <= 1) return;

    int next = 0;
    if (cur != -1) next = (cur + dir + cnt) % cnt;

    for (int i = 0; i < cnt; i++) XSetWindowBorder(dpy, wins[i], KOLOR_ZWYKLY);
    XSetInputFocus(dpy, wins[next], RevertToParent, CurrentTime);
    XSetWindowBorder(dpy, wins[next], KOLOR_FOCUS);
    XRaiseWindow(dpy, wins[next]);
    set_active_window(dpy, DefaultRootWindow(dpy), wins[next]);
}

// =============================================================
// MAIN INITIALIZATION & EVENT LOOP
// =============================================================
int main(void) {
    Display *dpy; Window root; XWindowAttributes attr; XButtonEvent start; XEvent ev;
    Cursor cursor; Atom wm_protocols, wm_delete_window; Window locked_window = None;

    if (!(dpy = XOpenDisplay(NULL))) return 1;
    signal(SIGCHLD, SIG_IGN); 
    XSetErrorHandler(xerror);
    int screen = DefaultScreen(dpy);
    root = DefaultRootWindow(dpy);

    wm_protocols = XInternAtom(dpy, "WM_PROTOCOLS", False);
    wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    net_wm_window_type = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
    net_wm_window_type_dock = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);

    setup_ewmh(dpy, root);

    cursor = XCreateFontCursor(dpy, XC_left_ptr);
    XDefineCursor(dpy, root, cursor);
    
    XSelectInput(dpy, root, SubstructureRedirectMask | SubstructureNotifyMask | PropertyChangeMask | EnterWindowMask);
    update_polybar_tags(dpy, root);

    // =========================================================
    // XGRABKEY SYSTEM (ALL DEFINITIONS FROM CONFIG.H)
    // =========================================================
    XGrabKey(dpy, XKeysymToKeycode(dpy, KLUCZ_TERMINAL), MOD_KEY, root,     False, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, KLUCZ_LAUNCHER), MOD_KEY, root,     False, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, KLUCZ_ZAMKNIJ), MOD_KEY, root,      False, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, KLUCZ_LOCK_FOCUS), MOD_KEY, root,   False, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, KLUCZ_FLOATING), MOD_KEY, root,     False, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, KLUCZ_FOCUS_NEXT), MOD_KEY, root,   False, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, KLUCZ_FOCUS_PREV), MOD_KEY, root,   False, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, KLUCZ_GAPS_PLUS), MOD_KEY, root,    False, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, KLUCZ_GAPS_MINUS), MOD_KEY, root,   False, GrabModeAsync, GrabModeAsync);

    XGrabKey(dpy, XKeysymToKeycode(dpy, KLUCZ_LAYOUT), MOD_KEY, root,                   False, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, KLUCZ_SCREENSHOT), MOD_KEY | ShiftMask, root,   False, GrabModeAsync, GrabModeAsync);
    
    XGrabKey(dpy, XKeysymToKeycode(dpy, KLUCZ_MASTER_MINUS), MOD_KEY | ShiftMask, root,     False, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, KLUCZ_MASTER_PLUS), MOD_KEY | ShiftMask, root,      False, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, KLUCZ_RESTART), MOD_KEY | ShiftMask, root,          False, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, KLUCZ_MAKSYMALIZUJ), MOD_KEY | ShiftMask, root,     False, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, KLUCZ_ROZMIAR_W), MOD_KEY | ShiftMask, root,        False, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, KLUCZ_ZAMKNIJ), MOD_KEY | ShiftMask, root,          False, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, KLUCZ_POMOCY), MOD_KEY | ShiftMask, root,           False, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, KLUCZ_PRINT_SCREEN), 0, root,                       False, GrabModeAsync, GrabModeAsync);

    KeySym tag_keys[] = { XK_1, XK_2, XK_3, XK_4, XK_5, XK_6, XK_7, XK_8, XK_9 };
    for (int i = 0; i < 9; i++) {
        XGrabKey(dpy, XKeysymToKeycode(dpy, tag_keys[i]), MOD_KEY, root, False, GrabModeAsync, GrabModeAsync);
        XGrabKey(dpy, XKeysymToKeycode(dpy, tag_keys[i]), MOD_KEY | ShiftMask, root, False, GrabModeAsync, GrabModeAsync);
    }

    XGrabKey(dpy, XKeysymToKeycode(dpy, XF86XK_AudioRaiseVolume), 0, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XF86XK_AudioLowerVolume), 0, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XF86XK_AudioMute), 0, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XF86XK_MonBrightnessUp), 0, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XF86XK_MonBrightnessDown), 0, root, True, GrabModeAsync, GrabModeAsync);

    XGrabButton(dpy, 1, MOD_KEY, root, True, ButtonPressMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(dpy, 3, MOD_KEY, root, True, ButtonPressMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);

    start.subwindow = None; start.button = 0;
    
    spawn(AUTOSTART_SCRIPT);

    while (1) {
        XNextEvent(dpy, &ev);

        if (ev.type == MapRequest) {
            XWindowAttributes wa;
            if (!XGetWindowAttributes(dpy, ev.xmaprequest.window, &wa)) continue;
            if (wa.override_redirect) continue;
            if (is_dock(dpy, ev.xmaprequest.window)) { XMapRaised(dpy, ev.xmaprequest.window); continue; }

            if (client_find(ev.xmaprequest.window)) continue;

            int help_win = is_help_window(dpy, ev.xmaprequest.window);
            int should_float = help_win || is_fullscreen_or_util(dpy, ev.xmaprequest.window, screen);
            
            client_add(ev.xmaprequest.window, active_tag, should_float);
            set_net_wm_desktop(dpy, ev.xmaprequest.window, active_tag);

            if (help_win) {
                int sw = DisplayWidth(dpy, screen);
                int sh = DisplayHeight(dpy, screen) - MARGINES_DOLNY;
                XMoveResizeWindow(dpy, ev.xmaprequest.window, (sw - 750) / 2, (sh - 600) / 2, 750, 600);
                XSetWindowBorderWidth(dpy, ev.xmaprequest.window, 1);
            }
            else if (should_float && wa.width >= DisplayWidth(dpy, screen)) {
                XMoveResizeWindow(dpy, ev.xmaprequest.window, 0, 0, DisplayWidth(dpy, screen), DisplayHeight(dpy, screen));
                XSetWindowBorderWidth(dpy, ev.xmaprequest.window, 0);
            } else {
                XSetWindowBackground(dpy, ev.xmaprequest.window, KOLOR_ZWYKLY);
                XClearWindow(dpy, ev.xmaprequest.window);
                XSetWindowBorderWidth(dpy, ev.xmaprequest.window, GRUBOSC_RAMKI);
            }

            XMapWindow(dpy, ev.xmaprequest.window);
            XSetWindowBorder(dpy, ev.xmaprequest.window, KOLOR_FOCUS);
            XSetInputFocus(dpy, ev.xmaprequest.window, RevertToParent, CurrentTime);
            set_active_window(dpy, root, ev.xmaprequest.window);
            XSelectInput(dpy, ev.xmaprequest.window, EnterWindowMask | LeaveWindowMask);

            apply_layout(dpy, screen);
            XFlush(dpy);
        }
        else if (ev.type == DestroyNotify) {
            if (ev.xdestroywindow.window == locked_window) locked_window = None;
            client_remove(ev.xdestroywindow.window);
            if (!is_dock(dpy, ev.xdestroywindow.window)) {
                apply_layout(dpy, screen);
                focus_fallback(dpy, root);
            }
        }
        else if (ev.type == UnmapNotify) {
            Client *c = client_find(ev.xunmap.window);
            if (!c) {
                if (!is_dock(dpy, ev.xunmap.window))
                    apply_layout(dpy, screen);
            }
            if (ev.xunmap.window == locked_window) locked_window = None;
        }
        else if (ev.type == ClientMessage) {
            if (ev.xclient.message_type == net_current_desktop) {
                int target_tag = ev.xclient.data.l[0] + 1;
                if (target_tag >= 1 && target_tag <= 9)
                    view_tag(dpy, screen, target_tag);
            }
        }
        else if (ev.type == ConfigureRequest) {
            XWindowChanges wc;
            wc.x = ev.xconfigurerequest.x; wc.y = ev.xconfigurerequest.y;
            wc.width = ev.xconfigurerequest.width; wc.height = ev.xconfigurerequest.height;
            wc.border_width = ev.xconfigurerequest.border_width;
            wc.sibling = ev.xconfigurerequest.above;
            wc.stack_mode = ev.xconfigurerequest.detail;
            XConfigureWindow(dpy, ev.xconfigurerequest.window, ev.xconfigurerequest.value_mask, &wc);
        }
        else if (ev.type == EnterNotify) {
            if (ev.xcrossing.window == root) {
                if (locked_window == None) {
                    XSetInputFocus(dpy, root, RevertToParent, CurrentTime);
                    set_active_window(dpy, root, None);
                }
            } else if (ev.xcrossing.window != locked_window && !is_dock(dpy, ev.xcrossing.window)) {
                Client *c = client_find(ev.xcrossing.window);
                if (c && c->tag == active_tag) {
                    XSetInputFocus(dpy, ev.xcrossing.window, RevertToParent, CurrentTime);
                    XSetWindowBorder(dpy, ev.xcrossing.window, KOLOR_FOCUS);
                    set_active_window(dpy, root, ev.xcrossing.window);
                }
            }
        }
        else if (ev.type == LeaveNotify && ev.xcrossing.window != root) {
            if (ev.xcrossing.window != locked_window && !is_dock(dpy, ev.xcrossing.window)) {
                XSetWindowBorder(dpy, ev.xcrossing.window, KOLOR_ZWYKLY);
            }
        }
        // =========================================================
        // KEYPRESS HANDLING (COMPLETELY REFACTORED SEPARATIONS)
        // =========================================================
        else if (ev.type == KeyPress) {
            KeySym base_ks = XLookupKeysym(&ev.xkey, 0);

            if (base_ks == KLUCZ_PRINT_SCREEN && ev.xkey.state == 0) { spawn(MOJ_SCREENSHOT); }
            else if (base_ks == KLUCZ_LAUNCHER && ev.xkey.state == MOD_KEY) { spawn(MOJ_LAUNCHER); }
            else if (base_ks == KLUCZ_TERMINAL && ev.xkey.state == MOD_KEY) { spawn(MOJ_TERMINAL); } 
            
            else if (base_ks == XF86XK_AudioRaiseVolume) { spawn("amixer sset Master 5%+"); }
            else if (base_ks == XF86XK_AudioLowerVolume) { spawn("amixer sset Master 5%-"); }
            else if (base_ks == XF86XK_AudioMute) { spawn("amixer sset Master toggle"); }
            else if (base_ks == XF86XK_MonBrightnessUp) { spawn("brightnessctl set +5%"); }
            else if (base_ks == XF86XK_MonBrightnessDown) { spawn("brightnessctl set 5%-"); }
            
            else if (base_ks == KLUCZ_RESTART && ev.xkey.state == (MOD_KEY | ShiftMask)) {
                execlp("micwm", "micwm", NULL); 
            }
            
            else if (base_ks == KLUCZ_LOCK_FOCUS && ev.xkey.state == MOD_KEY) {
                if (locked_window == None) {
                    Window focused; int revert_to; XGetInputFocus(dpy, &focused, &revert_to);
                    if (focused != root && focused != None) {
                        locked_window = focused;
                        XSetInputFocus(dpy, root, RevertToParent, CurrentTime);
                        XSetWindowBorder(dpy, locked_window, KOLOR_ZWYKLY);
                    }
                } else { locked_window = None; }
            }
            
            else if (base_ks == KLUCZ_GAPS_PLUS && ev.xkey.state == MOD_KEY) {
                gaps += 2; apply_layout(dpy, screen);
            }
            else if (base_ks == KLUCZ_GAPS_MINUS && ev.xkey.state == MOD_KEY) {
                gaps -= 2; if (gaps < 0) gaps = 0; apply_layout(dpy, screen);
            }
            else if (base_ks == KLUCZ_POMOCY && ev.xkey.state == (MOD_KEY | ShiftMask)) {
                spawn(POLECENIE_POMOCY);
            }
            
            else if (base_ks == KLUCZ_SCREENSHOT && ev.xkey.state == (MOD_KEY | ShiftMask)) {
                spawn(MOJ_SCREENSHOT);
            }
            else if (base_ks == KLUCZ_LAYOUT && ev.xkey.state == MOD_KEY) {
                current_layout = (current_layout + 1) % 2; 
                apply_layout(dpy, screen);
            }
            
            else if (base_ks == KLUCZ_FLOATING && ev.xkey.state == MOD_KEY) { toggle_floating(dpy, screen); }
            else if (base_ks == KLUCZ_FOCUS_NEXT && ev.xkey.state == MOD_KEY) { focus_stack(dpy, 1); }
            else if (base_ks == KLUCZ_FOCUS_PREV && ev.xkey.state == MOD_KEY) { focus_stack(dpy, -1); }
            
            else if (base_ks == KLUCZ_MASTER_MINUS && ev.xkey.state == (MOD_KEY | ShiftMask)) {
                master_fact -= 0.05; if (master_fact < 0.1) master_fact = 0.1; apply_layout(dpy, screen);
            }
            else if (base_ks == KLUCZ_MASTER_PLUS && ev.xkey.state == (MOD_KEY | ShiftMask)) {
                master_fact += 0.05; if (master_fact > 0.9) master_fact = 0.9; apply_layout(dpy, screen);
            }
            else if (base_ks >= XK_1 && base_ks <= XK_9) {
                int target_tag = base_ks - XK_1 + 1;
                if (ev.xkey.state == (MOD_KEY | ShiftMask)) {
                    Window focused; int rev; XGetInputFocus(dpy, &focused, &rev);
                    tag_window(dpy, screen, focused, target_tag);
                } else if (ev.xkey.state == MOD_KEY) {
                    view_tag(dpy, screen, target_tag);
                }
            }
            else if (base_ks == KLUCZ_ZAMKNIJ && ev.xkey.state == (MOD_KEY | ShiftMask)) break;
            
            else if (base_ks == KLUCZ_ZAMKNIJ && ev.xkey.subwindow != None) {
                if (!is_dock(dpy, ev.xkey.subwindow)) {
                    XEvent ke; ke.type = ClientMessage; ke.xclient.window = ev.xkey.subwindow;
                    ke.xclient.message_type = wm_protocols; ke.xclient.format = 32;
                    ke.xclient.data.l[0] = wm_delete_window; ke.xclient.data.l[1] = CurrentTime;
                    XSendEvent(dpy, ev.xkey.subwindow, False, NoEventMask, &ke);
                }
            }
            else if (base_ks == KLUCZ_MAKSYMALIZUJ && ev.xkey.state == (MOD_KEY | ShiftMask) && ev.xkey.subwindow != None) {
                if (!is_dock(dpy, ev.xkey.subwindow)) {
                    XMoveResizeWindow(dpy, ev.xkey.subwindow, 0, 0, DisplayWidth(dpy, screen), DisplayHeight(dpy, screen));
                    XSetWindowBorderWidth(dpy, ev.xkey.subwindow, 0);
                    XRaiseWindow(dpy, ev.xkey.subwindow);
                }
            }
            else if (base_ks == KLUCZ_ROZMIAR_W && ev.xkey.state == (MOD_KEY | ShiftMask) && ev.xkey.subwindow != None) {
                if (!is_dock(dpy, ev.xkey.subwindow)) {
                    XMoveResizeWindow(dpy, ev.xkey.subwindow,
                        (DisplayWidth(dpy, screen) - 800) / 2,
                        (DisplayHeight(dpy, screen) - 600) / 2, 800, 600);
                    XSetWindowBorderWidth(dpy, ev.xkey.subwindow, GRUBOSC_RAMKI);
                    XSetWindowBorder(dpy, ev.xkey.subwindow, KOLOR_FOCUS);
                }
            }
        }
        else if (ev.type == ButtonPress && ev.xbutton.subwindow != None) {
            if (is_dock(dpy, ev.xbutton.subwindow)) continue;
            if (ev.xbutton.subwindow != locked_window) {
                XGetWindowAttributes(dpy, ev.xbutton.subwindow, &attr);
                XSetInputFocus(dpy, ev.xbutton.subwindow, RevertToParent, CurrentTime);
                set_active_window(dpy, root, ev.xbutton.subwindow);
                XRaiseWindow(dpy, ev.xbutton.subwindow);
                if (attr.border_width > 0) {
                    start = ev.xbutton;
                    XSetWindowBorder(dpy, ev.xbutton.subwindow, KOLOR_FOCUS);
                } else { start.subwindow = None; }
            }
        }
        else if (ev.type == MotionNotify && start.subwindow != None) {
            int xdiff = ev.xbutton.x_root - start.x_root;
            int ydiff = ev.xbutton.y_root - start.y_root;
            int nx = attr.x, ny = attr.y, nw = attr.width, nh = attr.height;
            if (start.button == 1) {
                nx += xdiff; ny += ydiff;
                if (nx < 0) nx = 0; if (ny < 0) ny = 0;
                if (nx + nw + 2 * GRUBOSC_RAMKI > DisplayWidth(dpy, screen))
                    nx = DisplayWidth(dpy, screen) - nw - 2 * GRUBOSC_RAMKI;
                if (ny + nh + 2 * GRUBOSC_RAMKI > (DisplayHeight(dpy, screen) - MARGINES_DOLNY))
                    ny = (DisplayHeight(dpy, screen) - MARGINES_DOLNY) - nh - 2 * GRUBOSC_RAMKI;
            } else if (start.button == 3) {
                nw += xdiff; nh += ydiff;
                if (nw < MIN_ROZMIAR) nw = MIN_ROZMIAR;
                if (nh < MIN_ROZMIAR) nh = MIN_ROZMIAR;
                if (nx + nw + 2 * GRUBOSC_RAMKI > DisplayWidth(dpy, screen))
                    nw = DisplayWidth(dpy, screen) - nx - 2 * GRUBOSC_RAMKI;
                if (ny + nh + 2 * GRUBOSC_RAMKI > (DisplayHeight(dpy, screen) - MARGINES_DOLNY))
                    nh = (DisplayHeight(dpy, screen) - MARGINES_DOLNY) - ny - 2 * GRUBOSC_RAMKI;
            }
            XMoveResizeWindow(dpy, start.subwindow, nx, ny, nw, nh);
        }
        else if (ev.type == ButtonRelease && start.subwindow != None) {
            XSetWindowBorder(dpy, start.subwindow, KOLOR_ZWYKLY);
            start.subwindow = None;
        }
    }

    XFreeCursor(dpy, cursor);
    XCloseDisplay(dpy);
    return 0;
}