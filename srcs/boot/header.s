section .multiboot_header 
align 8 ; iso align sur 64
    header_start:
        dd 0xe85250d6
        dd 0
        dd header_end - header_start
        dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))
        dw 0
        dw 0
        dd 8
    header_end:

section .note.GNU-stack noalloc noexec nowrite progbits