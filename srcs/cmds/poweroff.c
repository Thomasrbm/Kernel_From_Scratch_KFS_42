#include "cmds.h"


void    cmd_poweroff()
{
    outw(0x604, 0x2000);
}
