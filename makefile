ASMFILES := $(shell find . -type f -name '*.s')
OBJFILES := $(ASMFILES:.s=.o)

kernel.iso: kernel grub.cfg
	mkdir -p iso/boot/grub
	cp kernel iso/boot/kernel
	cp grub.cfg iso/boot/grub/grub.cfg
	grub-mkrescue -o $@ iso
	rm -rf iso


kernel: $(OBJFILES)
	ld -melf_i386 -nostdlib -T linker.ld $^ -o $@

%.o: %.s
	nasm -f elf $^ -o $@

clean:
	rm -fv *.o
.PHONY: clean
