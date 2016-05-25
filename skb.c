#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "util.h"

#define CLEANMASK(mask)         (mask & ~(numlockmask|LockMask) & (ShiftMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask))
#define LENGTH(X)               (sizeof X / sizeof X[0])

typedef union {
	const char *m;
	const char **a;
} Arg;

typedef struct {
	unsigned int mod;
	KeySym keysym;
	void (*func)(const Arg *arg);
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

#include "config.skb.h"

static Key *mode = def;

void
grabkeys(void)
{
	updatenumlockmask();
        {
                unsigned int i, j;
                unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };
                KeyCode code;

                XUngrabKey(dpy, AnyKey, AnyModifier, root);
                for (i = 0; i < LENGTH(mode); i++)
                        if ((code = XKeysymToKeycode(dpy, mode[i].keysym)))
                                for (j = 0; j < LENGTH(modifiers); j++)
                                        XGrabKey(dpy, code, mode[i].mod | modifiers[j], root,
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
        for (i = 0; i < LENGTH(mode); i++)
                if (keysym == mode[i].keysym
                && CLEANMASK(mode[i].mod) == CLEANMASK(ev->state)
                && mode[i].func)
                        mode[i].func(&(mode[i].arg)); //this is where we would directly call spawn, rather than the funtable
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
	screen = DefaultScreen(dpy);
	root = DefaultRootWindow(dpy);
	grabkeys();
}
void
spawn(const Arg *arg)
{
        if (fork() == 0) {
                if (dpy)
                        close(ConnectionNumber(dpy));
                setsid();
                execvp(((char **)arg->a)[0], (char **)arg->a);
                fprintf(stderr, "skb: execvp %s", ((char **)arg->a)[0]);
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
                die("skb-"VERSION "\n");
        else if (argc != 1)
                die("usage: skb [-v]\n");
        if (!(dpy = XOpenDisplay(NULL)))
                die("dwm: cannot open display\n");
        setup();
        run();
        return EXIT_SUCCESS;
}

