bits 32
 
; load la nouvelle gdt
; flush les ancienc registres de section et les reload.

section .text
	global gdt_flush

gdt_flush:
	mov eax, [esp+4] ; la stack croit vers le bas donc -4.   elle push l arg puis push adresse retour. donc -4 -4.  -8.  faire +4 prend adresse premier arg 
	lgdt [eax]

	; les registres section fait 16 bits.


	mov ax, 0x10      ; sélecteur kernel data = index 2 × 8 = 0x10
	
	; avant la gdt grub est random. mtn elle est a adress 0x800

	; pointe tous au meme endroit, mais role different. read/write
	mov ds, ax        ;  data segment.  registre pointe vers le segment correct.  ex: ici 0x800 + 0x10 = kernel data.
	mov es, ax        ;  extra segment
	mov fs, ax        ;  segment général
	mov gs, ax        ;  segment général
	mov ss, ax        ;  stack segment

	; CS = exec kernell code

	

	;jump normal  → change juste EIP (adresse d'exécution)
	;far jump     → change EIP + CS (segment de code)

	jmp 0x08:flush
;  		^^^^  ^^^^^
;   	CS    EIP
	
flush:
	ret


section .note.GNU-stack noalloc noexec nowrite progbits