#include "handle_shell.h"


uint16_t	g_backbuffer[3][BUFFER_LINES * VGA_WIDHT];
int 		g_cursor_line[3] = {0, 0, 0};
int 		g_cursor_col[3]  = {0, 0, 0};
int 		g_view_offset[3] = {0, 0, 0}; // delimite ce qui est visible (les 24 lignes de l ecran parmi le back buffer.)
int			g_prompt_col = 0;
int			g_screen = 0;

// ligne de base 0 a 24
// g view offset = +1  a chaque descente hors du cadre
// 1 a 25 puis 2 a 26 etc


// sans ca curseur va  etre invisible au debut car la vga a ete remplit par grub donc le curseur en profite mais apres refresh g_screen y a plus les couleurs vga. car copie backbeffer dessur qui lui a pas de couleur sans ca.
static void	init_backbuffer(void) // sinon curseur sera noir sur noir.
{
	int	j;

	for (int i = 0; i < 3; i++)
	{
		j = 0;
		while (j < BUFFER_LINES * VGA_WIDHT)
		{
			g_backbuffer[i][j] = (WHITE_ON_BLACK << 8) | ' ';
			j++;
		}
	}
}

void	print_prompt()
{
	printk("kash> ", LIGHT_GREEN_ON_BLACK);
	g_prompt_col = g_cursor_col[g_screen];
}

void	welcome_prompt()
{
	printk("============== Welcome to Kash ==============\n\n", LIGHT_CYAN_ON_BLACK);
	printk("                 :::      ::::::::\n", LIGHT_CYAN_ON_BLACK);
	printk("               :+:      :+:    :+:\n", LIGHT_CYAN_ON_BLACK);
	printk("             +:+ +:+         +:+ \n",  LIGHT_CYAN_ON_BLACK);
	printk("           +#+  +:+       +#+\n", LIGHT_CYAN_ON_BLACK);
	printk("          +#+#+#+#+#+   +#+ \n",  LIGHT_CYAN_ON_BLACK);
	printk("               #+#    #+#\n",     LIGHT_CYAN_ON_BLACK);
	printk("              ###    ##########\n\n",  LIGHT_CYAN_ON_BLACK);
	printk("============== screen (%d) ==================\n\n", LIGHT_CYAN_ON_BLACK, g_screen);
	print_prompt(g_screen);
	refresh_g_screen(g_screen);
}

void handle_shell(void)
{
	uint8_t scancode;

	init_backbuffer();
	welcome_prompt();
	DumpHex((void*)0x800, sizeof(g_gdt));   // ce qui est à 0x800       // la source originale
	print_prompt();
	refresh_g_screen();
	while(1)
	{
		while (!(inb(KB_STATUS_PORT) & 0x01))
			;
		scancode = inb(KB_DATA_PORT);
		process_key(scancode);
	}
}