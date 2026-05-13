
#include "cmds.h"


void clear()
{
	for (int i = 0; i < (BUFFER_LINES * VGA_WIDHT); i++)
	{
		g_backbuffer[g_screen][i] = (WHITE_ON_BLACK << 8) | ' ';
	}
	g_cursor_col[g_screen] = 0;
	g_cursor_line[g_screen] = 0;
	welcome_prompt();
}