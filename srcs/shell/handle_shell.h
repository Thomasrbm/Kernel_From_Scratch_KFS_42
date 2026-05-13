#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h> 
#include "../cmds/cmds.h"
#include "../gdt/gdt.h"

// inline ca copie colle l instruction tel quel, sinon ca aligne + call + ret etc pas opti
// instruction : sortie : entree
// si : :  = sortie vide

// outb envoit vers un periph
static inline void outb(uint16_t port, uint8_t val)
{
    //      mov $0x3D4, %dx    ; port commande VGA
    //      mov $14, %al         ; registre 14 = byte haut du curseur
    //      outb %al, %dx        ; envoie
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port)); // la ligne qui sera copie colle, ca prend la AT&T, on peut forcer intel 
}


// inb recoit du periphe
static inline uint8_t inb(uint16_t port)
{
    // mov $0x60, %dx      ; port dans dx
    // inb %dx, %al        ; lis → résultat dans al 
    uint8_t ret;

    // on inverse ici car norme gcc sortie puis entre, comme nasm
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port)); // la ligne qui sera copie colle
    return (ret);
}


static inline void outw(uint16_t port, uint16_t val)
{
    __asm__ volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}


#define VGA_ADDR 0xB8000
#define VGA_WIDHT 80
#define VGA_HEIGHT 25


#define VGA_INDEX_PORT		0x3D4
#define VGA_DATA_PORT		0x3D5
#define VGA_CURSOR_HIGH	    14
#define VGA_CURSOR_LOW		15


#define KB_DATA_PORT		0x60
#define KB_STATUS_PORT		0x64
#define KB_RELEASE_MASK 	0x80


#define LEFT			0x4B
#define RIGHT			0x4D
#define UP				0x48
#define DOWN			0x50

#define WHITE_ON_BLACK        0x0F
#define LIGHT_GREEN_ON_BLACK  0x0A
#define LIGHT_CYAN_ON_BLACK   0x0B
#define YELLOW_ON_BLACK       0x0E
#define RED_ON_BLACK          0x04
#define LIGHT_RED_ON_BLACK    0x0C
#define MAGENTA_ON_BLACK      0x05
#define LIGHT_MAGENTA_ON_BLACK 0x0D
#define BLUE_ON_BLACK         0x01
#define LIGHT_BLUE_ON_BLACK   0x09
#define BROWN_ON_BLACK        0x06
#define LIGHT_GREY_ON_BLACK   0x07
#define DARK_GREY_ON_BLACK    0x08


#define BUFFER_LINES	      200

#define SCREEN_1    0x3B
#define SCREEN_2    0x3C
#define SCREEN_3    0x3D

extern uint16_t	g_backbuffer[3][BUFFER_LINES * VGA_WIDHT]; // copie invisible de tout ce qui est ecrit
extern int		g_cursor_line[3];
extern int		g_cursor_col[3];
extern int		g_view_offset[3];
extern int		g_prompt_col;
extern int      g_screen;


void update_cursor();
void newline();
char scancode_to_ascii(uint8_t scancode);
void newline();
void follow_cursor();
void update_cursor();
static void	init_backbuffer(void);
void    backbuffer_fill_char(int line , int col, char c, uint8_t color);
void printk(char *str, uint8_t color, ...);
void refresh_g_screen();
void	print_prompt();
void	welcome_prompt();
void handle_shell(void);
void process_key(uint8_t scancode);

void print_number(int nb, uint8_t color);
void backbuffer_putstr(char *str, uint8_t color);

int     strlenk(const char *s);
int     strcmpk(const char *a, const char *b);