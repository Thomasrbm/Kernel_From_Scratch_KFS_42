bits 32

extern handle_shell
extern gdt_init 

section .text
    global start 

start:
    mov esp, stack_top
    call gdt_init
    call handle_shell
    hlt


section .bss 
align 4 ; software norm convetion pour i386 voir lien.
    stack_bottom:
        resb 4096 * 4
    stack_top:



section .note.GNU-stack noalloc noexec nowrite progbits