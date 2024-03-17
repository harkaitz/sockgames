#ifndef PTERM_H
#define PTERM_H

#ifdef _WIN32
#  include <windows.h>
#  include <io.h>
#else
#  include <stdio.h>
#  include <unistd.h>
#  include <termios.h>
#endif


/* ----------------------------------------------------------------
 * ---- TERMINAL --------------------------------------------------
 * ---------------------------------------------------------------- */
static inline __attribute__((unused)) void
pterm_save_config(struct termios *_term) {
	tcgetattr(0, _term);
}

static inline __attribute__((unused)) void
pterm_restore_config(struct termios *_term)
{
	tcsetattr(0, TCSANOW, _term);
}

static inline __attribute__((unused)) void
pterm_set_gaming_mode(void)
{
	struct termios tio;
	tcgetattr(0, &tio);
	tio.c_lflag &=(~ICANON & ~ECHO);
	tcsetattr(0, TCSANOW, &tio);
}

#ifdef _WIN32
#  define fp_isatty(FP) _isatty(_fileno(FP))
#else
#  define fp_isatty(FP) isatty(fileno(FP))
#endif

/* ----------------------------------------------------------------
 * ---- ANSI COLORS -----------------------------------------------
 * ---------------------------------------------------------------- */

#define ANSI_TEXT(T) T "\x1b[0m"

#define ANSI_FG_WHITE()     "\x1b[97m"
#define ANSI_BG_WHITE()     "\x1b[107m"

#define ANSI_FG_RED_1()     "\x1b[31m"
#define ANSI_FG_GREEN_1()   "\x1b[32m"
#define ANSI_FG_YELLOW_1()  "\x1b[33m"
#define ANSI_FG_BLUE_1()    "\x1b[34m"
#define ANSI_FG_MAGENTA_1() "\x1b[35m"
#define ANSI_FG_CYAN_1()    "\x1b[36m"
#define ANSI_FG_GREY_1()    "\x1b[36m"

#define ANSI_FG_RED_2()     "\x1b[91m"
#define ANSI_FG_GREEN_2()   "\x1b[92m"
#define ANSI_FG_YELLOW_2()  "\x1b[93m"
#define ANSI_FG_BLUE_2()    "\x1b[94m"
#define ANSI_FG_MAGENTA_2() "\x1b[95m"
#define ANSI_FG_CYAN_2()    "\x1b[96m"
#define ANSI_FG_GREY_2()    "\x1b[96m"

#define ANSI_UNDERLINE() "\033[4m"
#define ANSI_BOLD()      "\033[1m"

#define ANSI_BG_RED_1()     "\x1b[41m"
#define ANSI_BG_GREEN_1()   "\x1b[42m"
#define ANSI_BG_YELLOW_1()  "\x1b[43m"
#define ANSI_BG_BLUE_1()    "\x1b[44m"
#define ANSI_BG_MAGENTA_1() "\x1b[45m"
#define ANSI_BG_CYAN_1()    "\x1b[46m"
#define ANSI_BG_GREY_1()    "\x1b[46m"

#define ANSI_BG_RED_2()     "\x1b[41;1m"
#define ANSI_BG_GREEN_2()   "\x1b[42;1m"
#define ANSI_BG_YELLOW_2()  "\x1b[43;1m"
#define ANSI_BG_BLUE_2()    "\x1b[44;1m"
#define ANSI_BG_MAGENTA_2() "\x1b[45;1m"
#define ANSI_BG_CYAN_2()    "\x1b[46;1m"
#define ANSI_BG_GREY_2()    "\x1b[46;1m"


#define ANSI_CLEAR_SCREEN()   "\033[2J"
#define ANSI_CLEAR_POSITION() "\033[0;0f"

#define PTERM_ARROW_UP()    "\033[A"
#define PTERM_ARROW_DOWN()  "\033[B"
#define PTERM_ARROW_RIGHT() "\033[C"
#define PTERM_ARROW_LEFT()  "\033[D"

#define PTERM_ESCAPE_CHAR '\033'
    
#endif
