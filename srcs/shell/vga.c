#include "handle_shell.h"


void update_cursor()
{
	int         real_line;
	uint16_t    pos;

	real_line = g_cursor_line[g_screen] - g_view_offset[g_screen];
	pos = real_line * VGA_WIDHT + g_cursor_col[g_screen];

	outb(VGA_INDEX_PORT, 14);
	outb(VGA_DATA_PORT, pos >> 8);
	outb(VGA_INDEX_PORT, 15);
	outb(VGA_DATA_PORT, pos & 0xFF); 
}


// chaque ligne passee derrier nous = 80 col (VGA_WIDGHT)  
// + la colone de la ligne actuelle

// stock la couleur + le char.
// 
void    backbuffer_fill_char(int line , int col, char c, uint8_t color)
{
	g_backbuffer[g_screen][line * VGA_WIDHT + col] = (uint16_t) color << 8 | c;
}

// while pour tout print
// si new line faire newline ft
// sinon
// print char 
// si jamais pointeur global curseur = fin de ligne (max widht) = equivalent de newline ecrit la suite sur newline.
void printk(char *str, uint8_t color, ...)
{
	va_list bonus_args;
	va_start(bonus_args, color);

	while (*str)
	{
		if (*str == '%')
		{
			str++;
			if (*str == 'd')
			{
				print_number(va_arg(bonus_args, int), YELLOW_ON_BLACK);
				str++;
				continue;
			}
			else if (*str == 'c')
			{
				backbuffer_fill_char(g_cursor_line[g_screen] ,g_cursor_col[g_screen], va_arg(bonus_args, int), YELLOW_ON_BLACK); // sinon warning dit que char devient int avec les variadique
				str++;
				continue;
			}
			else if (*str == 's')
			{
				backbuffer_putstr(va_arg(bonus_args, char *), YELLOW_ON_BLACK);
				str++;
				continue;
			}
			else 
				str--;
		}
		if (*str == '\n')
		{
			if (g_cursor_line[g_screen] == BUFFER_LINES)
				return ;
			newline();
		}
		else 
		{
			backbuffer_fill_char(g_cursor_line[g_screen], g_cursor_col[g_screen], *str, color);
			g_cursor_col[g_screen]++;
			if (g_cursor_col[g_screen] >= VGA_WIDHT)
				newline();
		}
		str++;
	}
}

void refresh_g_screen()
{
	uint16_t *vga;
	int     y;
	int     x;


	vga = (uint16_t *)VGA_ADDR;
	y = 0;
	while (y < VGA_HEIGHT)
	{
		x = 0;
		while (x < VGA_WIDHT)
		{
			vga[y * VGA_WIDHT + x] = g_backbuffer[g_screen][(y + g_view_offset[g_screen])  * VGA_WIDHT + x];
			x++;
		}
		y++;
	}
	update_cursor();
}