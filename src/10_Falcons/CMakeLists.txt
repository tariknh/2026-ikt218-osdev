########################################
# UiAOS - 10_Falcons
# Target: i386 / 32-bit kernel
########################################

cmake_minimum_required(VERSION 3.22.1)

set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

project(UiAOS LANGUAGES C CXX ASM_NASM)

file(LOCK ${CMAKE_SOURCE_DIR} DIRECTORY GUARD FILE)

set(OS_ARCH_TARGET "i386")
set(OS_KERNEL_BINARY "kernel.bin")
set(OS_KERNEL_IMAGE "kernel.iso")

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_ASM_NASM_SOURCE_FILE_EXTENSIONS "s;S;asm")
set(CMAKE_ASM_NASM_OBJECT_FORMAT "elf32")

set(CMAKE_ASM_NASM_COMPILE_OBJECT
    "<CMAKE_ASM_NASM_COMPILER> <DEFINES> <INCLUDES> <FLAGS> -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} -o <OBJECT> <SOURCE>"
)

set(OS_KERNEL_LINKER "${CMAKE_CURRENT_SOURCE_DIR}/src/arch/${OS_ARCH_TARGET}/linker.ld")

add_executable(uiaos-kernel
    src/multiboot2.asm
    src/kernel.c
    src/gdt.c
    src/gdt_flush.asm
    src/idt.c
    src/idt_load.asm
    src/isr.c
    src/isr.asm
    src/irq.c
    src/irq.asm
    src/keyboard.c
)

target_include_directories(uiaos-kernel PUBLIC include)

target_compile_options(uiaos-kernel PRIVATE
    $<$<COMPILE_LANGUAGE:C>:
        -Wall
        -Wextra
        -ffreestanding
        -nostdinc
        -nostdlib
        -fno-builtin
        -fno-stack-protector
        -fno-stack-check
        -fno-lto
        -fno-pie
        -m32
        -march=i386
        -mno-mmx
        -mno-sse
        -mno-sse2
        -mno-red-zone
        -Wno-main
        -Wno-unused-variable
        -Wno-unused-parameter
        -g
    >
    $<$<COMPILE_LANGUAGE:CXX>:
        -Wall
        -Wextra
        -ffreestanding
        -nostdinc
        -nostdlib
        -fno-builtin
        -fno-stack-protector
        -fno-stack-check
        -fno-lto
        -fno-pie
        -m32
        -march=i386
        -mno-mmx
        -mno-sse
        -mno-sse2
        -mno-red-zone
        -Wno-unused-variable
        -Wno-unused-parameter
        -g
    >
)

target_link_options(uiaos-kernel PRIVATE
    -m32
    -ffreestanding
    -nostdlib
    -fno-builtin
    -static
    -no-pie
    -T${OS_KERNEL_LINKER}
    -g
)

set_target_properties(uiaos-kernel PROPERTIES
    OUTPUT_NAME "${OS_KERNEL_BINARY}"
)

########################################
# Create Empty Fat32 Disk Image
########################################

set(DISK_IMAGE "${CMAKE_CURRENT_BINARY_DIR}/disk.iso")

add_custom_target(
    create-fat32-disk
    COMMAND dd if=/dev/zero of=${DISK_IMAGE} bs=1M count=32
    COMMAND mkfs.fat -F 32 ${DISK_IMAGE}
    VERBATIM
)

########################################
# OS Image Target
########################################

set(ISO_DIR ${CMAKE_CURRENT_BINARY_DIR}/iso)
set(LIMINE_CONFIG_FILE ${CMAKE_SOURCE_DIR}/limine.conf)
set(LIMINE_DIR /usr/local/limine)

add_custom_target(
    uiaos-create-image
    COMMAND rm -rf ${ISO_DIR}
	COMMAND mkdir -p ${ISO_DIR}
	COMMAND mkdir -p ${ISO_DIR}/boot

	# Kopier kernel og config til riktig sted
	COMMAND cp -v $<TARGET_FILE:uiaos-kernel> ${ISO_DIR}/boot/kernel.bin
	COMMAND cp -v ${LIMINE_CONFIG_FILE} ${ISO_DIR}/boot/limine.conf

	# Kopier Limine filer
	COMMAND cp -v ${LIMINE_DIR}/limine-bios.sys ${ISO_DIR}/
	COMMAND cp -v ${LIMINE_DIR}/limine-bios-cd.bin ${ISO_DIR}/
	COMMAND cp -v ${LIMINE_DIR}/limine-uefi-cd.bin ${ISO_DIR}/
        #${LIMINE_CONFIG_DIR}/limine.cfg
        #${LIMINE_DIR}/limine-bios.sys
        #${LIMINE_DIR}/limine-bios-cd.bin
        #${LIMINE_DIR}/limine-uefi-cd.bin
        #${ISO_DIR}/
    #COMMAND mkdir -p ${ISO_DIR}/EFI/BOOT
    #COMMAND cp -v ${LIMINE_DIR}/BOOTX64.EFI ${ISO_DIR}/EFI/BOOT/
    #COMMAND cp -v ${LIMINE_DIR}/BOOTIA32.EFI ${ISO_DIR}/EFI/BOOT/
    COMMAND xorriso -as mkisofs -b limine-bios-cd.bin
        -no-emul-boot -boot-load-size 4 -boot-info-table
        --efi-boot limine-uefi-cd.bin
        -efi-boot-part --efi-boot-image --protective-msdos-label
        ${ISO_DIR} -o ${CMAKE_CURRENT_BINARY_DIR}/${OS_KERNEL_IMAGE}
    COMMAND ${LIMINE_DIR}/limine bios-install ${CMAKE_CURRENT_BINARY_DIR}/${OS_KERNEL_IMAGE}
    DEPENDS create-fat32-disk uiaos-kernel
    VERBATIM
)
# Assignment 4 sources: memory manager, paging and PIT.
target_sources(uiaos-kernel PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/memory.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/paging.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pit.c
    src/song_player.c
    src/speaker.c
    src/piano.c
)
