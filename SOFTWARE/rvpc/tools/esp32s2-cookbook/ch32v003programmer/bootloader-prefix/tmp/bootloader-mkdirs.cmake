# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/kalata23/github/_espidf/esp-idf/components/bootloader/subproject"
  "/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/bootloader"
  "/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/bootloader-prefix"
  "/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/bootloader-prefix/tmp"
  "/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/bootloader-prefix/src/bootloader-stamp"
  "/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/bootloader-prefix/src"
  "/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
