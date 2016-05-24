#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define CLEANMASK(mask)         (mask & ~(numlockmask|LockMask) & (ShiftMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask))
#define LENGTH(X)               (sizeof X / sizeof X[0])

typedef union {
	        int i;
	        unsigned int ui;
	        float f;
	        const void *v;
} Arg;

typedef struct {
	        unsigned int mod;
	        KeySym keysym;
	        void (*func)(const Arg *);
	        const Arg arg;
} Key;

static void grabkeys(void);
static void keypress(XEvent *e);
static void run(void);
static void setup(void);
static void spawn(const Arg *arg);
static void updatenumlockmask(void);

/*variables*/
static int running = 1;
static Display *dpy;
static Window root;
static int screen;
static unsigned int numlockmask = 0;
static void (*handler[LASTEvent]) (XEvent *) = {
        [ButtonPress] = buttonpress,
        [ClientMessage] = clientmessage,
        [ConfigureRequest] = configurerequest,
        [ConfigureNotify] = configurenotify,
        [DestroyNotify] = destroynotify,
        [EnterNotify] = enternotify,
        [Expose] = expose,
        [FocusIn] = focusin,
        [KeyPress] = keypress,
        [MappingNotify] = mappingnotify,
        [MapRequest] = maprequest,
        [MotionNotify] = motionnotify,
        [PropertyNotify] = propertynotify,
        [UnmapNotify] = unmapnotify
};

#include "config.h"

void
grabkeys(void)
{
	updatenumlockmask();
        {
                unsigned int i, j;
                unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };
                KeyCode code;

                XUngrabKey(dpy, AnyKey, AnyModifier, root);
                for (i = 0; i < LENGTH(keys); i++)
                        if ((code = XKeysymToKeycode(dpy, keys[i].keysym)))
                                for (j = 0; j < LENGTH(modifiers); j++)
                                        XGrabKey(dpy, code, keys[i].mod | modifiers[j], root,
                                                 True, GrabModeAsync, GrabModeAsync);
        }
}

void
keypress(XEvent *e)
{
        unsigned int i;
        KeySym keysym;
        XKeyEvent *ev;

        ev = &e->xkey;
        keysym = XKeycodeToKeysym(dpy, (KeyCode)ev->keycode, 0);
        for (i = 0; i < LENGTH(keys); i++)
                if (keysym == keys[i].keysym
                && CLEANMASK(keys[i].mod) == CLEANMASK(ev->state)
                && keys[i].func)
                        keys[i].func(&(keys[i].arg)); //this is where we would directly call spawn, rather than the funtable
}

void
run(void)
{
        XEvent ev;
        /* main event loop */
        XSync(dpy, False);
        while (running && !XNextEvent(dpy, &ev))
               if (ev.type == KeyPress)
                        keypress(&ev); /* call handler */
}

void
setup(void)
{
	dpy = XOpenDisplay(NULL);
	screen = DefaultScreen(dpy);
	root = DefaultRootWindow(dpy);
	grabkeys();
void
spawn(const Arg *arg)
{
        if (arg->v == dmenucmd)
                dmenumon[0] = '0' + selmon->num;
        if (fork() == 0) {
                if (dpy)
                        close(ConnectionNumber(dpy));
                setsid();
                execvp(((char **)arg->v)[0], (char **)arg->v);
                fprintf(stderr, "dwm: execvp %s", ((char **)arg->v)[0]);
                perror(" failed");
                exit(EXIT_SUCCESS);
        }
}

void
updatenumlockmask(void)
{
        unsigned int i, j;
        XModifierKeymap *modmap;

        numlockmask = 0;
        modmap = XGetModifierMapping(dpy);
        for (i = 0; i < 8; i++)
                for (j = 0; j < modmap->max_keypermod; j++)
                        if (modmap->modifiermap[i * modmap->max_keypermod + j]
                           == XKeysymToKeycode(dpy, XK_Num_Lock))
                                numlockmask = (1 << i);
        XFreeModifiermap(modmap);
}

int
main(int argc, char *argv[])
{
        if (argc == 2 && !strcmp("-v", argv[1]))
                die("dwm-"VERSION "\n");
        else if (argc != 1)
                die("usage: dwm [-v]\n");
        if (!setlocale(LC_CTYPE, "") || !XSupportsLocale())
                fputs("warning: no locale support\n", stderr);
        if (!(dpy = XOpenDisplay(NULL)))
                die("dwm: cannot open display\n");
        setup();
        scan();
        run();
        cleanup();
        return EXIT_SUCCESS;
}

