#include "cmds.h"

void    cmd_halt()
{
    __asm__ volatile("cli; hlt");
}
