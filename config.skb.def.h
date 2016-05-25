#define MODKEY Mod1Mask
static const char *dmenucmd[] = { "dmenu_run", NULL };
static const char *termcmd[] = { "st", NULL };
//change to implement commands directly, no union stuff
static Key def[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_z,      spawn,          {.a = dmenucmd} },
	{ MODKEY|ShiftMask,             XK_Return, spawn,          {.a = termcmd} },
};
