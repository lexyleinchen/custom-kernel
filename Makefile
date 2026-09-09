TARGET = x86_64-elf

CC = $(TARGET)-gcc
CXX = $(TARGET)-g++
LD = $(TARGET)-ld
AS = nasm
APP_DIRS := $(wildcard src/apps/*)
APP_SOURCES := $(foreach dir,$(APP_DIRS),$(wildcard $(dir)/*.cpp))
APP_OBJECTS := $(patsubst src/%.cpp,build/%.o,$(APP_SOURCES))

CFLAGS = -ffreestanding \
	-fno-stack-protector \
	-fno-pie \
	-mno-red-zone \
	-Wall \
	-Wextra

CXXFLAGS = -ffreestanding \
	-fno-stack-protector \
	-fno-rtti \
	-fno-pie \
	-mno-red-zone \
	-Wall \
	-Wextra

LDFLAGS = -T src/linker.ld

KERNEL = build/kernel.bin

all: $(KERNEL)

build:
	mkdir -p build

build/kernel.o: src/kernel/kernel.c | build
	$(CC) $(CFLAGS) -Isrc/kernel -Isrc/os -c $< -o $@

build/log.o: src/kernel/core/log.c | build
	$(CC) $(CFLAGS) -Isrc/kernel -c $< -o $@

build/framebuffer.o: src/kernel/framebuffer/framebuffer.c | build
	$(CC) $(CFLAGS) -Isrc/kernel -c $< -o $@

build/block.o: src/kernel/storage/block.c | build
	$(CC) $(CFLAGS) -Isrc/kernel -c $< -o $@

build/storage.o: src/kernel/storage/storage.c | build
	$(CC) $(CFLAGS) -Isrc/kernel -c $< -o $@

build/partition.o: src/kernel/storage/partition/partition.c | build
	$(CC) $(CFLAGS) -Isrc/kernel -c $< -o $@

build/multiboot.o: src/kernel/boot/multiboot.c | build
	$(CC) $(CFLAGS) -Isrc/kernel -c $< -o $@

build/ps2.o: src/kernel/drivers/ps2/ps2.c | build
	$(CC) $(CFLAGS) -Isrc/kernel -c $< -o $@

build/pci.o: src/kernel/drivers/pci/pci.c | build
	$(CC) $(CFLAGS) -Isrc/kernel -c $< -o $@

build/usb.o: src/kernel/drivers/usb/usb.c | build
	$(CC) $(CFLAGS) -Isrc/kernel -c $< -o $@

build/ahci.o: src/kernel/drivers/ahci/ahci.c | build
	$(CC) $(CFLAGS) -Isrc/kernel -c $< -o $@

build/ide.o: src/kernel/drivers/ide/ide.c | build
	$(CC) $(CFLAGS) -Isrc/kernel -c $< -o $@

build/mouse.o: src/kernel/inputs/mouse.c | build
	$(CC) $(CFLAGS) -Isrc/kernel -c $< -o $@

build/boot.o: src/boot/boot.asm | build
	$(AS) -f elf64 $< -o $@

build/os.o: src/os/os.cpp | build
	$(CXX) $(CXXFLAGS) -Isrc/os -Isrc/kernel -c $< -o $@

build/graphics.o: src/os/graphics.cpp | build
	$(CXX) $(CXXFLAGS) -Isrc/os -c $< -o $@

build/ui.o: src/os/ui.cpp | build
	$(CXX) $(CXXFLAGS) -Isrc/os -c $< -o $@

build/font.o: src/os/font.cpp | build
	$(CXX) $(CXXFLAGS) -Isrc/os -c $< -o $@

build/taskbar.o: src/os/taskbar.cpp | build
	$(CXX) $(CXXFLAGS) -Isrc/os -c $< -o $@

build/os_mouse.o: src/os/os_mouse.cpp | build
	$(CXX) $(CXXFLAGS) -Isrc/os -c $< -o $@

build/%.o: src/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -Isrc/os -Isrc/kernel -Isrc/apps -c $< -o $@

$(KERNEL): build/boot.o \
		build/kernel.o \
		build/log.o \
		build/framebuffer.o \
		build/block.o \
		build/storage.o \
		build/partition.o \
		build/multiboot.o \
		build/ps2.o \
		build/pci.o \
		build/usb.o \
		build/ahci.o \
		build/ide.o \
		build/mouse.o \
		build/os.o \
		build/graphics.o \
		build/ui.o \
		build/font.o \
		build/taskbar.o \
		build/os_mouse.o \
		$(APP_OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

iso: $(KERNEL)
	mkdir -p iso/boot/grub
	cp $(KERNEL) iso/boot/kernel.bin
	grub2-mkrescue -o build/PrintOS.iso iso

clean:
	rm -rf build
	rm -rf iso/boot/kernel.bin

.PHONY: all iso clean
