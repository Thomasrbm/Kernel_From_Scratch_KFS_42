#include "gdt.h"


// global pour que pas de copie a chaque appel etc. une duree de vie tout le kernell

static t_gdt_entry g_gdt[7];
static t_gdt_ptr g_gdt_ptr;




void set_gdt_entry(int index, int base, int limit, int access, int limit_flags)
{
	// 32bits recu pour chaque
	// 0xAABBCCDD => on veut CCDD donc un taille 16 donc a droite
	g_gdt[index].limit = limit & 0xFFFF; // garde que les 16 de droite ( du bas )
	g_gdt[index].low_base = base & 0xFFFF; // garde CCDD
	g_gdt[index].mid_base = (base >> 16)& 0xFF; // BB dans taille 8
	g_gdt[index].access = access;

	// 0xAABBC  . on veut A 
	g_gdt[index].limit_flags = (limit >> 16) & 0x0F | (limit_flags) << 4;  // bit A de droite restant de limit  + // recoit 4 bit dans limit flags. (ex 0xC) . sur meme ligne sinon result ecrase
	g_gdt[index].high_base = (base >> 24) & 0xFF; //  AA
}


// limit flags.  4 bit du haut pour les flat.  1 ou 0 pour presence .  ET la fin de la limit.  la limit fait 20.  16 dans limit et 4 dernier dans limit flag.

// bit 7  bit 6  bit 5  bit 4  		 bit 3  bit 2  bit 1  bit 0
//   G     DB      L    AVL    		  L19    L18    L17    L16
// |_________flags________| 	 	|________limite 16..19_____|



// access

// bit 7  bit 6  bit 5  bit 4  bit 3  bit 2  bit 1  bit 0
//   P     DPL    DPL    S      type   type   type   type
// |__P__|_____DPL_____|__S__|_____________type____________|

// P = ce segment exite et est valide. sinon cpu exception
// DPL = lvl de privilege  . 00 = ring 0 = kernel  , 11 = ring 3 = user  ( 1 et 2 unused)
// S = 1 si segment memoire, 0 pour truc CPU

// type :   bit 2  bit 1  bit 0
//			  E      W      A
// 		   expand  write  accessed

// le 3 : 0 = data segment,  1 = code segment


// EXEMPLE : 

// 0x9A kernel code :
// bit 7  bit 6  bit 5  bit 4  bit 3  bit 2  bit 1  bit 0
//   P     DPL    DPL    S      type   type   type   type
//   1      0      0     1       1      0      1      0
// |__P__|_____DPL_____|__S__|_____________type____________|


//0xFA user code :
// bit 7  bit 6  bit 5  bit 4  bit 3  bit 2  bit 1  bit 0
//   P     DPL    DPL    S      type   type   type   type
//   1      1      1     1       1      0      1      0
// |__P__|_____DPL_____|__S__|_____________type____________|

void gdt_init()
{
	// 0xFFFFF = 1048575 pages 1page = 4096bytes = 4GB de RAM en taille
	
	// 0xC = 1100 = granualrite a 1 = en page donc GB sinon active en bit donc 1Mb
	// DB = 1, segmet 32bit
	// l = mode long desactive
	// avl = truc d os jais pas compris.

	set_gdt_entry(0, 0, 0, 	  	 0x00, 0x0);
	set_gdt_entry(1, 0, 0xFFFFF, 0x9A, 0xC);
	set_gdt_entry(2, 0, 0xFFFFF, 0x92, 0xC);
	set_gdt_entry(3, 0, 0xFFFFF, 0x92, 0xC);
	set_gdt_entry(4, 0, 0xFFFFF, 0xFA, 0xC);
	set_gdt_entry(5, 0, 0xFFFFF, 0xF2, 0xC);
	set_gdt_entry(6, 0, 0xFFFFF, 0xF2, 0xC);


	g_gdt_ptr.base = 0x00000800;
	g_gdt_ptr.limit = sizeof(g_gdt) - 1; // limit = dernier bit valide, pas la taille. ex 56 taille = 0 a 55. 55 dernier bit valide

	ft_memcpy((void*)0x00000800, g_gdt, sizeof(g_gdt)); // place vraiment a l adresse

	gdt_flush(&g_gdt_ptr);
}	