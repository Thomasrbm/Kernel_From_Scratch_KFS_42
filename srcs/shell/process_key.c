#include "handle_shell.h"

void scroll_up()
{
	if (g_view_offset[g_screen] > 0)
		g_view_offset[g_screen]--;
}

void scroll_down()
{
	if (g_view_offset[g_screen] < g_cursor_line[g_screen] - VGA_HEIGHT + 1)
		g_view_offset[g_screen]++;
}

void	handle_backspace()
{
	if ((g_cursor_col[g_screen] + 1 == VGA_WIDHT) && ((g_backbuffer[g_screen][g_cursor_line[g_screen] * VGA_WIDHT + g_cursor_col[g_screen]] & 0xFF) != ' '))
		backbuffer_fill_char(g_cursor_line[g_screen], g_cursor_col[g_screen], ' ', WHITE_ON_BLACK);
	else if (g_cursor_col[g_screen] > g_prompt_col)
	{
		g_cursor_col[g_screen]--;
		backbuffer_fill_char(g_cursor_line[g_screen], g_cursor_col[g_screen], ' ', WHITE_ON_BLACK);
	}
}

bool check_after_line()
{
	int number_of_spaces = 0;
	int space_after_cursor =  VGA_WIDHT - g_cursor_col[g_screen] + 1;
	int temp_cursor = g_cursor_col[g_screen];
	while (temp_cursor <= VGA_WIDHT)
	{
		if ((g_backbuffer[g_screen][g_cursor_line[g_screen] * VGA_WIDHT + temp_cursor + 1] & 0xFF) == ' ')
			number_of_spaces++;
		temp_cursor++;
	}
	if ((number_of_spaces != space_after_cursor) || ((g_backbuffer[g_screen][g_cursor_line[g_screen] * VGA_WIDHT + g_cursor_col[g_screen]] & 0xFF) != ' ')) // soit pas que des space soit actuellement sur lettre
		return true ;
	return false;
}

void handle_screen(int scancode)
{
	if (scancode ==  SCREEN_1)
	{
		g_screen = 0;
	}
	else if (scancode ==  SCREEN_2)
	{
		g_screen = 1;
		if (g_cursor_line[g_screen] == 0)
			welcome_prompt();
	}
	else if (scancode ==  SCREEN_3)
	{
		g_screen = 2;
		if (g_cursor_line[g_screen] == 0)
			welcome_prompt();
	}
}

bool handle_cmd(char * tmp_cmd)
{
	if (!strcmpk(tmp_cmd, "clear"))
	{
		clear();
		return true;
	}	
	if (!strcmpk(tmp_cmd, "poweroff"))
	{
		cmd_poweroff();
		return true;
	}	
	if (!strcmpk(tmp_cmd, "reboot"))
	{
		cmd_reboot();
		return true;
	}	
	if (!strcmpk(tmp_cmd, "help"))
	{
		cmd_help();
		return true;
	}	
	if (!strcmpk(tmp_cmd, "hlt"))
	{
		cmd_halt();
		return true;
	}	
	return false;
}

void    handle_enter()
{
    char tmp_cmd[10];
    int tmp_cursor; 
    tmp_cursor = g_prompt_col;
    int i = 0;
    while ((g_backbuffer[g_screen][g_cursor_line[g_screen] * VGA_WIDHT + tmp_cursor] & 0xFF) != ' ')
    {
        tmp_cmd[i] = (g_backbuffer[g_screen][g_cursor_line[g_screen] * VGA_WIDHT + tmp_cursor] & 0xFF);
        i++;
        tmp_cursor++;
    }
    tmp_cmd[i] = '\0';
	if (handle_cmd(tmp_cmd))
		return ;
	else
	{
		if (g_cursor_line[g_screen] == BUFFER_LINES)
			return ;
		newline();
		print_prompt();
	}
}

void process_key(uint8_t scancode)
{   
	char c = scancode_to_ascii(scancode);

	if (scancode & KB_RELEASE_MASK)
		return ;
	if (c == '\b')
		handle_backspace();
	else if (scancode ==  SCREEN_1 || scancode ==  SCREEN_3 || scancode ==  SCREEN_2)
		handle_screen(scancode);
	else if (c == '\n')
		handle_enter();
	else if (scancode == LEFT)
	{
		if (g_cursor_col[g_screen] > g_prompt_col)
			g_cursor_col[g_screen]--;
	}
	else if (scancode == RIGHT)
	{
		if (g_cursor_col[g_screen] < VGA_WIDHT && check_after_line())
			g_cursor_col[g_screen]++;
	}
	else if (scancode == UP)
		scroll_up();
	else if (scancode == DOWN)
		scroll_down();
	else if (c) // si ou non sur la map key_board
	{
		if (g_cursor_col[g_screen] < VGA_WIDHT)
		{
			backbuffer_fill_char(g_cursor_line[g_screen], g_cursor_col[g_screen], c, WHITE_ON_BLACK);
			if (g_cursor_col[g_screen] + 1 != VGA_WIDHT)
				g_cursor_col[g_screen]++;
		}
	}
	refresh_g_screen();
}