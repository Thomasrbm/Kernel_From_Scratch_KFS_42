#include "handle_shell.h"

void follow_cursor()
{
	int first_line = g_view_offset[g_screen];
	int last_line = VGA_HEIGHT + g_view_offset[g_screen] - 1;

	if (g_cursor_line[g_screen] > last_line)
		g_view_offset[g_screen] = g_cursor_line[g_screen] - VGA_HEIGHT + 1;
	if (g_cursor_line[g_screen] < first_line)
		g_view_offset[g_screen] =  g_cursor_line[g_screen];
}


void newline()
{
	g_cursor_line[g_screen]++;
	g_cursor_col[g_screen] = 0;
	follow_cursor();
	refresh_g_screen(g_screen);
}

void backbuffer_putstr(char *str, uint8_t color)
{
	int i = 0;

	while (str[i])
	{
		backbuffer_fill_char(g_cursor_line[g_screen] ,g_cursor_col[g_screen], str[i], color);
		g_cursor_col[g_screen]++;
		i++;
	}
}

void print_number(int nb, uint8_t color)
{
	if (nb == -2147483648)
	{
		backbuffer_putstr("-2147483648", color);
		return ;
	}
	if (nb < 0)
	{
		backbuffer_fill_char(g_cursor_line[g_screen], g_cursor_col[g_screen], '-', color);
		g_cursor_col[g_screen]++;
		nb *= -1;
	}
	if (nb >= 10)
		print_number(nb / 10, color);
	backbuffer_fill_char(g_cursor_line[g_screen], g_cursor_col[g_screen], (nb % 10) + '0', color);
	g_cursor_col[g_screen]++;
}


int     strlenk(const char *s)
{
	int i;
	i = 0;
	while (s[i])
		i++;
	return i;
}

int     strcmpk(const char *a, const char *b)
{
	int i;
	i = 0;
	while (a[i] && a[i] == b[i])
		i++;
	return a[i] - b[i];
}
