ASMFILES := $(shell find . -type f -name '*.s')
OBJFILES := $(ASMFILES:.s=.o)

qemu: kernel.iso
	qemu-system-x86_64 $<
.PHONY: qemu

kernel.iso: kernel grub.cfg
	mkdir -p iso/boot/grub
	cp kernel iso/boot/kernel
	cp grub.cfg iso/boot/grub/grub.cfg
	grub-mkrescue -o $@ iso

kernel: linker.ld $(OBJFILES)
	ld -melf_i386 -nostdlib -T linker.ld $(OBJFILES) -o $@

%.o: %.s
	nasm -f elf $^ -o $@

clean:
	find . -type f -name '*.o' | xargs rm -f
	rm -f kernel.iso
	rm -f kernel
	rm -rf iso/
.PHONY: clean
