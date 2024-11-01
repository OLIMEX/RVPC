file(REMOVE_RECURSE
  "bootloader/bootloader.bin"
  "bootloader/bootloader.elf"
  "bootloader/bootloader.map"
  "config/sdkconfig.cmake"
  "config/sdkconfig.h"
  "flash_project_args"
  "project_elf_src_esp32.c"
  "usb_sandbox.bin"
  "usb_sandbox.map"
  "CMakeFiles/usb_sandbox.elf.dir/project_elf_src_esp32.c.obj"
  "CMakeFiles/usb_sandbox.elf.dir/project_elf_src_esp32.c.obj.d"
  "project_elf_src_esp32.c"
  "usb_sandbox.elf"
  "usb_sandbox.elf.pdb"
)

# Per-language clean rules from dependency scanning.
foreach(lang C)
  include(CMakeFiles/usb_sandbox.elf.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
