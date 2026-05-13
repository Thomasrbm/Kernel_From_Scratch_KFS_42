#include "cmds.h"

void    cmd_reboot()
{
    __asm__ volatile("cli");
    outb(0x64, 0xFE);
}