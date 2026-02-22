#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "config.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

int xerror(Display *dpy, XErrorEvent *ee) { return 0; }

int main(void) {
    Display *dpy;
    Window root, bar;
    XWindowAttributes attr;
    XButtonEvent start;
    XEvent ev;
    Cursor cursor;
    GC gc;
    char status_text[256];
    
    strncpy(status_text, TEKST_NA_PASKU, sizeof(status_text) - 1);
    signal(SIGCHLD, SIG_IGN);

    if(!(dpy = XOpenDisplay(NULL))) return 1;
    XSetErrorHandler(xerror);

    int screen = DefaultScreen(dpy);
    root = DefaultRootWindow(dpy);

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

    XGrabKey(dpy, XKeysymToKeycode(dpy, XK_t), Mod1Mask, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XK_q), Mod1Mask, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XK_q), Mod1Mask | ShiftMask, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XK_f), Mod1Mask | ShiftMask, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XK_w), Mod1Mask | ShiftMask, root, True, GrabModeAsync, GrabModeAsync);

    XGrabButton(dpy, 1, Mod1Mask, root, True, ButtonPressMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(dpy, 3, Mod1Mask, root, True, ButtonPressMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);

    start.subwindow = None;
    start.button = 0;

    if(system(AUTOSTART_SCRIPT)) {}

    while(1) {
        XNextEvent(dpy, &ev);
        XRaiseWindow(dpy, bar);

        if(ev.type == Expose && ev.xexpose.window == bar) {
            XClearWindow(dpy, bar);
            XDrawString(dpy, bar, gc, 10, 14, status_text, strlen(status_text));
        }
        else if(ev.type == PropertyNotify && ev.xproperty.window == root && ev.xproperty.atom == XA_WM_NAME) {
            char *name = NULL;
            if(XFetchName(dpy, root, &name) && name) {
                strncpy(status_text, name, sizeof(status_text) - 1);
                status_text[sizeof(status_text) - 1] = '\0';
                XFree(name);
            }
            XClearWindow(dpy, bar);
            XDrawString(dpy, bar, gc, 10, 14, status_text, strlen(status_text));
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
        else if(ev.type == KeyPress) {
            KeySym ks = XLookupKeysym(&ev.xkey, 0);
            
            if(ks == XK_q && (ev.xkey.state & ShiftMask)) break;

            else if(ks == XK_q && ev.xkey.subwindow != None && ev.xkey.subwindow != bar) {
                XKillClient(dpy, ev.xkey.subwindow);
            }
            else if(ks == XK_f && (ev.xkey.state & ShiftMask) && ev.xkey.subwindow != None && ev.xkey.subwindow != bar) {
                XMoveResizeWindow(dpy, ev.xkey.subwindow, 0, WYSOKOSC_PASKA, 
                                 DisplayWidth(dpy, screen), DisplayHeight(dpy, screen) - WYSOKOSC_PASKA);
                XSetWindowBorderWidth(dpy, ev.xkey.subwindow, 0);
                XRaiseWindow(dpy, ev.xkey.subwindow);
            }
            else if(ks == XK_w && (ev.xkey.state & ShiftMask) && ev.xkey.subwindow != None && ev.xkey.subwindow != bar) {
                XMoveResizeWindow(dpy, ev.xkey.subwindow, (DisplayWidth(dpy, screen)-800)/2, (DisplayHeight(dpy, screen)-600)/2, 800, 600);
                XSetWindowBorderWidth(dpy, ev.xkey.subwindow, GRUBOSC_RAMKI);
                XSetWindowBorder(dpy, ev.xkey.subwindow, KOLOR_FOCUS);
            }
            else if(ks == XK_t) {
                if(fork() == 0) {
                    close(ConnectionNumber(dpy));
                    execlp(MOJ_TERMINAL, MOJ_TERMINAL, NULL);
                    _exit(0);
                }
            }
        }
        else if(ev.type == ButtonPress && ev.xbutton.subwindow != None && ev.xbutton.subwindow != bar) {
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
        else if(ev.type == MotionNotify && start.subwindow != None) {
            int xdiff = ev.xbutton.x_root - start.x_root;
            int ydiff = ev.xbutton.y_root - start.y_root;
            XMoveResizeWindow(dpy, start.subwindow,
                attr.x + (start.button == 1 ? xdiff : 0),
                attr.y + (start.button == 1 ? ydiff : 0),
                MAX(MIN_ROZMIAR, attr.width + (start.button == 3 ? xdiff : 0)),
                MAX(MIN_ROZMIAR, attr.height + (start.button == 3 ? ydiff : 0)));
        }
        else if(ev.type == ButtonRelease && start.subwindow != None) {
            XSetWindowBorder(dpy, start.subwindow, KOLOR_ZWYKLY);
            start.subwindow = None;
        }
    }

    XFreeGC(dpy, gc);
    XFreeCursor(dpy, cursor);
    XCloseDisplay(dpy);
    return 0;
}