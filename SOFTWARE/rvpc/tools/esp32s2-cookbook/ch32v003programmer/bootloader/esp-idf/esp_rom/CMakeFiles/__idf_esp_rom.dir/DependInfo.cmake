
# Consider dependencies only in project.
set(CMAKE_DEPENDS_IN_PROJECT_ONLY OFF)

# The set of languages for which implicit dependencies are needed:
set(CMAKE_DEPENDS_LANGUAGES
  "ASM"
  )
# The set of files for implicit dependencies of each language:
set(CMAKE_DEPENDS_CHECK_ASM
  "/home/kalata23/github/esp-idf_5_1/esp-idf/components/esp_rom/patches/esp_rom_longjmp.S" "/home/kalata23/github/esp32s2-cookbook/ch32v003programmer/bootloader/esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_longjmp.S.obj"
  )
set(CMAKE_ASM_COMPILER_ID "GNU")

# Preprocessor definitions for this target.
set(CMAKE_TARGET_DEFINITIONS_ASM
  "BOOTLOADER_BUILD=1"
  "ESP_PLATFORM"
  "IDF_VER=\"v5.1-rc2-4-gc570f67461\""
  "SOC_MMU_PAGE_SIZE=CONFIG_MMU_PAGE_SIZE"
  "_GNU_SOURCE"
  )

# The include file search paths:
set(CMAKE_ASM_TARGET_INCLUDE_PATH
  "config"
  "/home/kalata23/github/esp-idf_5_1/esp-idf/components/esp_rom/include"
  "/home/kalata23/github/esp-idf_5_1/esp-idf/components/esp_rom/include/esp32s2"
  "/home/kalata23/github/esp-idf_5_1/esp-idf/components/esp_rom/esp32s2"
  "/home/kalata23/github/esp-idf_5_1/esp-idf/components/log/include"
  "/home/kalata23/github/esp-idf_5_1/esp-idf/components/esp_common/include"
  "/home/kalata23/github/esp-idf_5_1/esp-idf/components/esp_hw_support/include"
  "/home/kalata23/github/esp-idf_5_1/esp-idf/components/esp_hw_support/include/soc"
  "/home/kalata23/github/esp-idf_5_1/esp-idf/components/esp_hw_support/include/soc/esp32s2"
  "/home/kalata23/github/esp-idf_5_1/esp-idf/components/esp_hw_support/port/esp32s2/."
  "/home/kalata23/github/esp-idf_5_1/esp-idf/components/esp_hw_support/port/esp32s2/private_include"
  "/home/kalata23/github/esp-idf_5_1/esp-idf/components/newlib/platform_include"
  "/home/kalata23/github/esp-idf_5_1/esp-idf/components/xtensa/include"
  "/home/kalata23/github/esp-idf_5_1/esp-idf/components/xtensa/esp32s2/include"
  "/home/kalata23/github/esp-idf_5_1/esp-idf/components/soc/include"
  "/home/kalata23/github/esp-idf_5_1/esp-idf/components/soc/esp32s2"
  "/home/kalata23/github/esp-idf_5_1/esp-idf/components/soc/esp32s2/include"
  "/home/kalata23/github/esp-idf_5_1/esp-idf/components/hal/esp32s2/include"
  "/home/kalata23/github/esp-idf_5_1/esp-idf/components/hal/include"
  "/home/kalata23/github/esp-idf_5_1/esp-idf/components/hal/platform_port/include"
  )

# The set of dependency files which are needed:
set(CMAKE_DEPENDS_DEPENDENCY_FILES
  "/home/kalata23/github/esp-idf_5_1/esp-idf/components/esp_rom/esp32s2/usb_descriptors.c" "esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/esp32s2/usb_descriptors.c.obj" "gcc" "esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/esp32s2/usb_descriptors.c.obj.d"
  "/home/kalata23/github/esp-idf_5_1/esp-idf/components/esp_rom/patches/esp_rom_crc.c" "esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_crc.c.obj" "gcc" "esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_crc.c.obj.d"
  "/home/kalata23/github/esp-idf_5_1/esp-idf/components/esp_rom/patches/esp_rom_efuse.c" "esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_efuse.c.obj" "gcc" "esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_efuse.c.obj.d"
  "/home/kalata23/github/esp-idf_5_1/esp-idf/components/esp_rom/patches/esp_rom_regi2c_esp32s2.c" "esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_regi2c_esp32s2.c.obj" "gcc" "esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_regi2c_esp32s2.c.obj.d"
  "/home/kalata23/github/esp-idf_5_1/esp-idf/components/esp_rom/patches/esp_rom_spiflash.c" "esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_spiflash.c.obj" "gcc" "esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_spiflash.c.obj.d"
  "/home/kalata23/github/esp-idf_5_1/esp-idf/components/esp_rom/patches/esp_rom_sys.c" "esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_sys.c.obj" "gcc" "esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_sys.c.obj.d"
  "/home/kalata23/github/esp-idf_5_1/esp-idf/components/esp_rom/patches/esp_rom_systimer.c" "esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_systimer.c.obj" "gcc" "esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_systimer.c.obj.d"
  "/home/kalata23/github/esp-idf_5_1/esp-idf/components/esp_rom/patches/esp_rom_uart.c" "esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_uart.c.obj" "gcc" "esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_uart.c.obj.d"
  )

# Targets to which this target links.
set(CMAKE_TARGET_LINKED_INFO_FILES
  "/home/kalata23/github/esp32s2-cookbook/ch32v003programmer/bootloader/esp-idf/log/CMakeFiles/__idf_log.dir/DependInfo.cmake"
  "/home/kalata23/github/esp32s2-cookbook/ch32v003programmer/bootloader/esp-idf/esp_common/CMakeFiles/__idf_esp_common.dir/DependInfo.cmake"
  "/home/kalata23/github/esp32s2-cookbook/ch32v003programmer/bootloader/esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/DependInfo.cmake"
  "/home/kalata23/github/esp32s2-cookbook/ch32v003programmer/bootloader/esp-idf/xtensa/CMakeFiles/__idf_xtensa.dir/DependInfo.cmake"
  "/home/kalata23/github/esp32s2-cookbook/ch32v003programmer/bootloader/esp-idf/soc/CMakeFiles/__idf_soc.dir/DependInfo.cmake"
  "/home/kalata23/github/esp32s2-cookbook/ch32v003programmer/bootloader/esp-idf/hal/CMakeFiles/__idf_hal.dir/DependInfo.cmake"
  "/home/kalata23/github/esp32s2-cookbook/ch32v003programmer/bootloader/esp-idf/efuse/CMakeFiles/__idf_efuse.dir/DependInfo.cmake"
  "/home/kalata23/github/esp32s2-cookbook/ch32v003programmer/bootloader/esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/DependInfo.cmake"
  "/home/kalata23/github/esp32s2-cookbook/ch32v003programmer/bootloader/esp-idf/spi_flash/CMakeFiles/__idf_spi_flash.dir/DependInfo.cmake"
  "/home/kalata23/github/esp32s2-cookbook/ch32v003programmer/bootloader/esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/DependInfo.cmake"
  "/home/kalata23/github/esp32s2-cookbook/ch32v003programmer/bootloader/esp-idf/micro-ecc/CMakeFiles/__idf_micro-ecc.dir/DependInfo.cmake"
  "/home/kalata23/github/esp32s2-cookbook/ch32v003programmer/bootloader/esp-idf/esp_app_format/CMakeFiles/__idf_esp_app_format.dir/DependInfo.cmake"
  )

# Fortran module output directory.
set(CMAKE_Fortran_TARGET_MODULE_DIR "")
