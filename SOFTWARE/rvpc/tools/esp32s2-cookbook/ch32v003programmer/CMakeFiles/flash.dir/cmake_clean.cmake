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
  "CMakeFiles/flash"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/flash.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
