/* See LICENSE file for copyright and license details. */

/* appearance */
#define USE_ANTIALIASING
//static char font[] = "fonts/LiberationMono-Regular.ttf:fonts/LiberationMono-Bold.ttf";
//static char font[] = "fonts/inconsolata-dz.ttf:fonts/inconsolata-dz.ttf";
//static char font[] = "fonts/monaco.ttf:fonts/monaco.ttf";
//static char font[] = "fonts/TerminusTTF-4.46.0.ttf:fonts/TerminusTTF-4.46.0.ttf";
static char font[] = "fonts/FIXED_V0.TTF:fonts/FIXED_V0.TTF";
static int fontsize = 8;
static int borderpx = 2;
static int initial_width = 320;
static int initial_height = 240;
static char shell[] = "/bin/sh";

/* double-click timeout (in milliseconds) between clicks for selection */
static unsigned int doubleclicktimeout = 300;
static unsigned int tripleclicktimeout = 600;

/* TERM value */
static char termname[] = "xterm";

static unsigned int tabspaces = 4;
#define WORD_BREAK " "

/* Terminal colors (16 first used in escape sequence) */
SDL_Color colormap[] = {
	/* 8 normal colors */
	{   0,   0,   0, 0 },//black
	{ 128,   0,   0, 0 },//"red3",
	{   0, 128,   0, 0 },//"green3",
	{ 128, 128,   0, 0 },//"yellow3",
	{   0,   0, 128, 0 },//"blue2",
	{ 128,   0, 128, 0 },//"magenta3",
	{   0, 128, 128, 0 },//"cyan3",
	{ 192, 192, 192, 0 },//"gray90",

	/* 8 bright colors */
	{ 128, 128, 128, 0}, //"gray50",
	{ 255,   0,   0, 0 },//red
	{   0, 255,   0, 0 },//green
	{ 255, 255,   0, 0 },//"yellow",
	{   0,   0, 255, 0 },//"#0000ff",
	{ 255,   0, 255, 0 },//"magenta",
	{   0, 255, 255, 0 },//"cyan",
	{ 255, 255, 255, 0 },//"white",

	[255] = { 0, 0, 0, 0 },

	/* more colors can be added after 255 to use with DefaultXX */
	{ 204, 204, 204, 0},
	{  51,  51,  51, 0},
};

/*
 * Default colors (colorname index)
 * foreground, background, cursor, unfocused cursor
 */
static unsigned int defaultfg = 7;
static unsigned int defaultbg = 0;
static unsigned int defaultcs = 256;
static unsigned int defaultucs = 257;

/*
 * Special keys (change & recompile st.info accordingly)
 * Keep in mind that kpress() in st.c hardcodes some keys.
 *
 * Mask value:
 * * Use XK_ANY_MOD to match the key no matter modifiers state
 * * Use XK_NO_MOD to match the key alone (no modifiers)
 */

/* key, mask, output */
static Key key[] = {
	{ SDLK_LEFT,      KMOD_ALT,  "\033[1;3D" },
	{ SDLK_RIGHT,     KMOD_ALT,  "\033[1;3C" },

	{ SDLK_BACKSPACE, 0, "\177" },
	{ SDLK_INSERT,    0, "\033[2~" },
	{ SDLK_DELETE,    0, "\033[3~" },
	{ SDLK_HOME,      0, "\033[1~" },
	{ SDLK_END,       0, "\033[4~" },
	{ SDLK_PAGEUP,    0, "\033[5~" },
	{ SDLK_PAGEDOWN,  0, "\033[6~" },
	{ SDLK_F1,        0, "\033OP"   },
	{ SDLK_F2,        0, "\033OQ"   },
	{ SDLK_F3,        0, "\033OR"   },
	{ SDLK_F4,        0, "\033OS"   },
	{ SDLK_F5,        0, "\033[15~" },
	{ SDLK_F6,        0, "\033[17~" },
	{ SDLK_F7,        0, "\033[18~" },
	{ SDLK_F8,        0, "\033[19~" },
	{ SDLK_F9,        0, "\033[20~" },
	{ SDLK_F10,       0, "\033[21~" },
	{ SDLK_F11,       0, "\033[23~" },
	{ SDLK_F12,       0, "\033[24~" },
};

/* Internal shortcuts. */
#define MODKEY KMOD_ALT

static Shortcut shortcuts[] = {
	/* modifier		key		function	argument */
	{ MODKEY|KMOD_SHIFT,	SDLK_PAGEUP,	xzoom,		{.i = +1} },
	{ MODKEY|KMOD_SHIFT,	SDLK_PAGEDOWN,	xzoom,		{.i = -1} },
};

