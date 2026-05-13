ASM_SOURCES := $(shell find srcs -name '*.s')
C_SOURCES := $(shell find srcs -name '*.c')

OBJS := $(patsubst srcs/%.s, objs/%.o, $(ASM_SOURCES))
OBJS += $(patsubst srcs/%.c, objs/%.o, $(C_SOURCES))

DEPS := $(patsubst objs/%.o, objs/%.d, $(filter objs/shell/%.o, $(OBJS)))

ISO = distro/x86_64/kernel.iso

CFLAGS := -m32 -ffreestanding -fno-builtin -fno-exceptions \
          -fno-stack-protector -nostdlib -nodefaultlibs

-include $(DEPS)

objs/%.o: srcs/%.s
	@mkdir -p $(dir $@)
	nasm -f elf32 $< -o $@

objs/%.o: srcs/%.c
	@mkdir -p $(dir $@)
	gcc $(CFLAGS) -MMD -MP -c $< -o $@

build-x86_64: $(ISO)

$(ISO): $(OBJS)
	mkdir -p distro/x86_64
	ld -m elf_i386 -n -o distro/x86_64/kernel.bin -T targets/linker.ld $(OBJS)
	cp distro/x86_64/kernel.bin targets/iso/boot/kernel.bin
	grub-mkrescue /usr/lib/grub/i386-pc -o $(ISO) targets/iso

run:
	env -i PATH=/usr/bin:/bin HOME=$(HOME) DISPLAY=$(DISPLAY) \
	XAUTHORITY=$(XAUTHORITY) qemu-system-i386 -cdrom $(ISO)

fclean:
	rm -rf objs/ distro/
	rm -f targets/iso/boot/kernel.bin

re: fclean build-x86_64

.PHONY: build-x86_64 run fclean re