#pragma once

#include <stdint.h>
#include "../utils/utils.h"
#include "../shell/handle_shell.h"



typedef struct s_gdt_entry
{
	uint16_t	limit;
	uint16_t	low_base;
	uint8_t		mid_base;
	uint8_t		access;
	uint8_t		limit_flags;
	uint8_t		high_base;
} __attribute__((packed)) t_gdt_entry;

typedef struct s_gdt_ptr
{
	uint16_t limit;
	uint32_t base;
} __attribute__((packed)) t_gdt_ptr;


void gdt_flush(t_gdt_ptr *ptr);

static t_gdt_entry g_gdt[7];
static t_gdt_ptr g_gdt_ptr;
void    DumpHex(const void *data, int size);