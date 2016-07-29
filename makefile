CXX := clang++
LD := ld
AS := nasm

MODULES := kernel.bin
COMMON_OBJFILES += src/common/common.o src/common/string.o \
                   src/common/interrupts.o

COMMON_FLAGS += -fPIC -ffreestanding -fno-builtin \
                -fno-exceptions -fno-rtti \
                -MMD -mno-sse -mno-mmx -mno-red-zone -Wall -Wextra -Werror \
                -pedantic -Wshadow -Wpointer-arith -Wcast-align \
                -Wwrite-strings -Wredundant-decls -Wno-unused-parameter \
                -Winline -Wno-long-long -Wuninitialized \
                -Wno-unused-private-field \
                -Wno-gnu-zero-variadic-macro-arguments

CXXFLAGS += $(COMMON_FLAGS) \
            -std=c++14 \
            -I src/ \
            -I src/modules/kernel \
            -I src/include

ifneq (,$(findstring clang,$(CXX)))
# we are using clang, add clang-specific flags
CXXFLAGS += --target=x86_64-pc-none-elf -Wno-gnu-zero-variadic-macro-arguments
endif

HFILES = $(shell echo $(find src/ -type f -name '*.h') $(find src/include))
CXXFILES = $(shell find src/ -type f -name '*.cpp')
DEPFILES = $(CFILES:.c=.d) $(CXXFILES:.cpp=.d)

LDFLAGS := -nostdlib

# always run clang-format regardless
$(shell clang-format -i $(CFILES) $(CXXFILES) $(HFILES))

all: kernel.iso
.PHONY: all

-include $(DEPFILES)

qemu: kernel.iso
	qemu-system-x86_64 $<
.PHONY: qemu

bochs: kernel.iso bochsrc.txt
	bochs
.PHONY: qemu

kernel.iso: start grub.cfg $(MODULES)
	mkdir -p iso/boot/grub
	cp start iso/boot/start
	cp grub.cfg iso/boot/grub/grub.cfg
	cp $(MODULES) iso/boot/
	grub-mkrescue -o $@ iso

start: start.ld src/start.o
	$(LD) -T $^ $(LDFLAGS) -o $@

%.o: %.s
	$(AS) -f elf64 $^ -o $@

%.bin: %.elf
	objcopy -O binary $< $@

KERNEL_DEPS = module.ld src/modules/kernel/entry.o src/modules/kernel/main.o \
            src/modules/kernel/assert.o \
            src/modules/kernel/boot/multiboot_info.o \
            src/modules/kernel/cmos.o \
            src/modules/kernel/cpuid.o \
            src/modules/kernel/halt.o \
            src/modules/kernel/idt.o \
            src/modules/kernel/interrupt_stubs.o \
            src/modules/kernel/ioapic.o \
            src/modules/kernel/irq.o \
            src/modules/kernel/isr.o \
            src/modules/kernel/kernel.o \
            src/modules/kernel/keyboard.o \
            src/modules/kernel/lapic.o \
            src/modules/kernel/msr.o \
            src/modules/kernel/pci.o \
            src/modules/kernel/pic.o \
            src/modules/kernel/pit.o \
            src/modules/kernel/register.o \
            src/modules/kernel/terminal.o \
            src/modules/kernel/util/emb_list_impl.o \
            src/modules/kernel/util/string_util.o \
            src/modules/kernel/vm/frame_pool.o \
            src/modules/kernel/vm/memory_manager.o \
            src/modules/kernel/vm/page_table.o \
            src/modules/kernel/vm/pager.o \
            src/modules/kernel/vm/pallocator.o \
            src/modules/kernel/vm/pde.o \
            src/modules/kernel/vm/pdpe.o \
            src/modules/kernel/vm/pml4e.o \
            src/modules/kernel/vm/pte.o \
            src/modules/kernel/vm/vm.o \
            $(COMMON_OBJFILES)

TEST_DEPS = $(KERNEL_DEPS) \
            src/test/test_main.o \
            src/test/test.o \
            src/test/test_example.o \
            src/test/util/test_emb_list.o

kernel.elf: $(KERNEL_DEPS)
	$(LD) -T $^ $(LDFLAGS) -o $@

# same number of characters
tstknl.elf: $(TEST_DEPS)
	$(LD) -T $^ $(LDFLAGS) -o $@

test: COMMON_FLAGS += -D RUN_TESTS -I src/test
test: test_kernel.iso
	qemu-system-x86_64 -m 16g test_kernel.iso

test_kernel.iso: start grub.cfg tstknl.bin
	mkdir -p iso/boot/grub
	cp start iso/boot/start
	cp test_grub.cfg iso/boot/grub/grub.cfg
	cp tstknl.bin iso/boot/
	grub-mkrescue -o $@ iso

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	find src/ -type f \( -name '*.o' -o -name '*.d' \) | xargs rm -fv
	rm -fv *.iso *.elf *.bin
	rm -fv start
	rm -rfv iso/
.PHONY: clean
