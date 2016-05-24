#define MODKEY Mod1Mask
static Arg dmenucmd = { "dmenu_run", NULL };
static Arg termcmd = { "st", NULL };
//change to implement commands directly, no union stuff
static Key def[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_z,      Spawn,          dmenucmd},
	{ MODKEY|ShiftMask,             XK_Return, Spawn,          termcmd},
};
