/* ansi.h: ANSI terminal escape sequences */

#ifndef _ANSITERM_H_
#define _ANSITERM_H_

#include <stdio.h>

#define ansiterm_reset		"\033[0m"
#define ansiterm_bold		"\033[1m"
#define ansiterm_underl		"\033[4m"

#define ansiterm_red		"\x1b[0;31m"
#define ansiterm_green		"\x1b[0;33m"
#define ansiterm_blue		"\x1b[0;36m"
#define ansiterm_yellow		"\x1b[0;33;02m"

#endif
