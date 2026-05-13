#include "gdt.h"

static void	print_hex_byte(unsigned char byte)
{
	const char	*hex = "0123456789ABCDEF";
	char		buf[3];

	buf[0] = hex[byte >> 4];
	buf[1] = hex[byte & 0xF];
	buf[2] = '\0';
	printk("%s", WHITE_ON_BLACK, buf);
}

static void	dump_end_line(char *ascii, int i)
{
	int	j;

	ascii[(i + 1) % 8] = '\0';
	j = (i + 1) % 8;
	while (j < 8)
	{
		printk("   ", WHITE_ON_BLACK);
		j++;
	}
	printk("| %s \n", WHITE_ON_BLACK, ascii);
}

static void	dump_byte(char *ascii, unsigned char byte, int i)
{
	print_hex_byte(byte);
	printk(" ", WHITE_ON_BLACK);
	ascii[i % 8] = (byte >= ' ' && byte <= '~') ? byte : '.';
}

void    DumpHex(const void *data, int size)
{
    int             i;
    unsigned char   byte;

    i = 0;
    while (i < size)
    {
        byte = ((unsigned char *)data)[i];
        print_hex_byte(byte);
        printk(" ", WHITE_ON_BLACK);
        if ((i + 1) % 8 == 0)
            printk("\n", WHITE_ON_BLACK);
        i++;
    }
    if (size % 8 != 0)
        printk("\n", WHITE_ON_BLACK);
}