#!/bin/bash
ELF="$1"
OUT="$2"

SIZE=$(stat -c %s "$ELF")

echo '#include <stdint.h>' > "$OUT"
echo "const uint32_t shell_elf_size = $SIZE;" >> "$OUT"

cp "$ELF" /tmp/shell_elf.elf
xxd -i /tmp/shell_elf.elf | sed 's/_tmp_shell_elf_elf/shell_elf/g' >> "$OUT"
rm /tmp/shell_elf.elf