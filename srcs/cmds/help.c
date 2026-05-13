
#include "cmds.h"

void    cmd_help()
{
    g_cursor_col[g_screen] = g_prompt_col;
    printk("Available commands:\n", MAGENTA_ON_BLACK);
    printk("help     - show this help\n", LIGHT_MAGENTA_ON_BLACK);
    printk("clear    - clear the screen\n", LIGHT_MAGENTA_ON_BLACK);
    printk("reboot   - reboot the machine\n", LIGHT_MAGENTA_ON_BLACK);
    printk("halt     - halt the CPU\n", LIGHT_MAGENTA_ON_BLACK);
    printk("poweroff - power off the machine\n", LIGHT_MAGENTA_ON_BLACK);
    print_prompt();
}


  
  