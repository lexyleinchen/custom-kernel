# Details

Date : 2026-09-10 19:28:09

Directory /home/lexyleinchen/osdev/PrintOS

Total : 62 files,  5651 codes, 11 comments, 1541 blanks, all 7203 lines

[Summary](results.md) / Details / [Diff Summary](diff.md) / [Diff Details](diff-details.md)

## Files
| filename | language | code | comment | blank | total |
| :--- | :--- | ---: | ---: | ---: | ---: |
| [Makefile](/Makefile) | Makefile | 108 | 0 | 36 | 144 |
| [README.md](/README.md) | Markdown | 106 | 0 | 68 | 174 |
| [iso/boot/grub/grub.cfg](/iso/boot/grub/grub.cfg) | Properties | 4 | 0 | 0 | 4 |
| [src/apps/calculator/calculator.cpp](/src/apps/calculator/calculator.cpp) | C++ | 9 | 0 | 4 | 13 |
| [src/apps/calculator/calculator.h](/src/apps/calculator/calculator.h) | C++ | 6 | 0 | 2 | 8 |
| [src/apps/diskmanager/diskmanager.cpp](/src/apps/diskmanager/diskmanager.cpp) | C++ | 264 | 0 | 57 | 321 |
| [src/apps/diskmanager/diskmanager.h](/src/apps/diskmanager/diskmanager.h) | C++ | 6 | 0 | 2 | 8 |
| [src/apps/filebrowser/filebrowser.cpp](/src/apps/filebrowser/filebrowser.cpp) | C++ | 178 | 1 | 45 | 224 |
| [src/apps/filebrowser/filebrowser.h](/src/apps/filebrowser/filebrowser.h) | C++ | 6 | 0 | 2 | 8 |
| [src/apps/terminal/terminal.cpp](/src/apps/terminal/terminal.cpp) | C++ | 105 | 0 | 35 | 140 |
| [src/apps/terminal/terminal.h](/src/apps/terminal/terminal.h) | C++ | 7 | 0 | 3 | 10 |
| [src/apps/texteditor/texteditor.c](/src/apps/texteditor/texteditor.c) | C | 9 | 0 | 4 | 13 |
| [src/apps/texteditor/texteditor.h](/src/apps/texteditor/texteditor.h) | C++ | 6 | 0 | 2 | 8 |
| [src/boot/boot.asm](/src/boot/boot.asm) | x86 and x86_64 Assembly | 105 | 4 | 40 | 149 |
| [src/kernel/boot/multiboot.c](/src/kernel/boot/multiboot.c) | C | 47 | 0 | 9 | 56 |
| [src/kernel/boot/multiboot.h](/src/kernel/boot/multiboot.h) | C++ | 11 | 0 | 5 | 16 |
| [src/kernel/core/log.c](/src/kernel/core/log.c) | C | 112 | 0 | 27 | 139 |
| [src/kernel/core/log.h](/src/kernel/core/log.h) | C++ | 14 | 0 | 8 | 22 |
| [src/kernel/core/work.c](/src/kernel/core/work.c) | C | 74 | 0 | 19 | 93 |
| [src/kernel/core/work.h](/src/kernel/core/work.h) | C++ | 27 | 0 | 13 | 40 |
| [src/kernel/drivers/ahci/ahci.c](/src/kernel/drivers/ahci/ahci.c) | C | 47 | 0 | 5 | 52 |
| [src/kernel/drivers/ahci/ahci.h](/src/kernel/drivers/ahci/ahci.h) | C++ | 12 | 0 | 6 | 18 |
| [src/kernel/drivers/ide/ide.c](/src/kernel/drivers/ide/ide.c) | C | 292 | 0 | 68 | 360 |
| [src/kernel/drivers/ide/ide.h](/src/kernel/drivers/ide/ide.h) | C++ | 12 | 0 | 6 | 18 |
| [src/kernel/drivers/pci/pci.c](/src/kernel/drivers/pci/pci.c) | C | 97 | 0 | 19 | 116 |
| [src/kernel/drivers/pci/pci.h](/src/kernel/drivers/pci/pci.h) | C++ | 24 | 0 | 9 | 33 |
| [src/kernel/drivers/ps2/ps2.c](/src/kernel/drivers/ps2/ps2.c) | C | 141 | 0 | 36 | 177 |
| [src/kernel/drivers/ps2/ps2.h](/src/kernel/drivers/ps2/ps2.h) | C++ | 14 | 0 | 8 | 22 |
| [src/kernel/drivers/usb/usb.c](/src/kernel/drivers/usb/usb.c) | C | 725 | 0 | 167 | 892 |
| [src/kernel/drivers/usb/usb.h](/src/kernel/drivers/usb/usb.h) | C++ | 30 | 0 | 7 | 37 |
| [src/kernel/framebuffer/framebuffer.c](/src/kernel/framebuffer/framebuffer.c) | C | 8 | 0 | 3 | 11 |
| [src/kernel/framebuffer/framebuffer.h](/src/kernel/framebuffer/framebuffer.h) | C++ | 18 | 0 | 7 | 25 |
| [src/kernel/inputs/keyboard.c](/src/kernel/inputs/keyboard.c) | C | 0 | 0 | 1 | 1 |
| [src/kernel/inputs/keyboard.h](/src/kernel/inputs/keyboard.h) | C++ | 15 | 0 | 9 | 24 |
| [src/kernel/inputs/mouse.c](/src/kernel/inputs/mouse.c) | C | 169 | 0 | 46 | 215 |
| [src/kernel/inputs/mouse.h](/src/kernel/inputs/mouse.h) | C++ | 24 | 0 | 14 | 38 |
| [src/kernel/kernel.c](/src/kernel/kernel.c) | C | 29 | 0 | 3 | 32 |
| [src/kernel/storage/block.c](/src/kernel/storage/block.c) | C | 70 | 0 | 23 | 93 |
| [src/kernel/storage/block.h](/src/kernel/storage/block.h) | C++ | 41 | 0 | 15 | 56 |
| [src/kernel/storage/partition/filesystem/fat32/fat32.c](/src/kernel/storage/partition/filesystem/fat32/fat32.c) | C | 932 | 0 | 256 | 1,188 |
| [src/kernel/storage/partition/filesystem/fat32/fat32.h](/src/kernel/storage/partition/filesystem/fat32/fat32.h) | C++ | 52 | 0 | 17 | 69 |
| [src/kernel/storage/partition/filesystem/filesystem.c](/src/kernel/storage/partition/filesystem/filesystem.c) | C | 64 | 0 | 20 | 84 |
| [src/kernel/storage/partition/filesystem/filesystem.h](/src/kernel/storage/partition/filesystem/filesystem.h) | C++ | 32 | 0 | 14 | 46 |
| [src/kernel/storage/partition/partition.c](/src/kernel/storage/partition/partition.c) | C | 373 | 0 | 105 | 478 |
| [src/kernel/storage/partition/partition.h](/src/kernel/storage/partition/partition.h) | C++ | 39 | 0 | 17 | 56 |
| [src/kernel/storage/storage.c](/src/kernel/storage/storage.c) | C | 107 | 0 | 31 | 138 |
| [src/kernel/storage/storage.h](/src/kernel/storage/storage.h) | C++ | 16 | 0 | 10 | 26 |
| [src/linker.ld](/src/linker.ld) | LinkerScript | 32 | 0 | 7 | 39 |
| [src/os/app.h](/src/os/app.h) | C++ | 8 | 0 | 3 | 11 |
| [src/os/app\_registry.h](/src/os/app_registry.h) | C++ | 24 | 0 | 9 | 33 |
| [src/os/font.cpp](/src/os/font.cpp) | C++ | 446 | 0 | 61 | 507 |
| [src/os/font.h](/src/os/font.h) | C++ | 7 | 0 | 5 | 12 |
| [src/os/graphics.cpp](/src/os/graphics.cpp) | C++ | 65 | 1 | 18 | 84 |
| [src/os/graphics.h](/src/os/graphics.h) | C++ | 17 | 0 | 10 | 27 |
| [src/os/os.cpp](/src/os/os.cpp) | C++ | 36 | 2 | 6 | 44 |
| [src/os/os.h](/src/os/os.h) | C++ | 11 | 0 | 5 | 16 |
| [src/os/os\_mouse.cpp](/src/os/os_mouse.cpp) | C++ | 48 | 0 | 6 | 54 |
| [src/os/os\_mouse.h](/src/os/os_mouse.h) | C++ | 5 | 0 | 3 | 8 |
| [src/os/taskbar.cpp](/src/os/taskbar.cpp) | C++ | 36 | 2 | 8 | 46 |
| [src/os/taskbar.h](/src/os/taskbar.h) | C++ | 6 | 0 | 3 | 9 |
| [src/os/ui.cpp](/src/os/ui.cpp) | C++ | 284 | 1 | 76 | 361 |
| [src/os/ui.h](/src/os/ui.h) | C++ | 39 | 0 | 18 | 57 |

[Summary](results.md) / Details / [Diff Summary](diff.md) / [Diff Details](diff-details.md)