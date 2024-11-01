# Install script for directory: /home/kalata23/github/_espidf/esp-idf

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/home/kalata23/.espressif/tools/xtensa-esp32-elf/esp-2022r1-11.2.0/xtensa-esp32-elf/bin/xtensa-esp32-elf-objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/xtensa/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/esp_ringbuf/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/efuse/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/driver/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/esp_pm/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/mbedtls/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/esp_app_format/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/bootloader_support/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/bootloader/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/esptool_py/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/partition_table/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/esp_partition/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/app_update/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/spi_flash/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/pthread/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/esp_system/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/esp_rom/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/hal/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/log/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/heap/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/soc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/esp_hw_support/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/freertos/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/newlib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/cxx/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/esp_common/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/esp_timer/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/app_trace/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/esp_event/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/nvs_flash/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/esp_phy/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/vfs/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/lwip/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/esp_netif/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/wpa_supplicant/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/esp_wifi/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/bt/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/unity/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/cmock/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/console/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/http_parser/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/esp-tls/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/esp_adc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/esp_eth/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/esp_gdbstub/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/esp_hid/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/tcp_transport/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/esp_http_client/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/esp_http_server/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/esp_https_ota/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/esp_https_server/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/esp_lcd/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/protobuf-c/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/protocomm/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/esp_local_ctrl/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/esp_psram/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/espcoredump/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/wear_levelling/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/sdmmc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/fatfs/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/idf_test/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/ieee802154/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/json/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/mqtt/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/openthread/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/perfmon/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/spiffs/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/ulp/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/usb/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/wifi_provisioning/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/main/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/kalata23/SVN/ESP32-SBC-FabGL/Software/trunk/Production_Test/tools/esp32s2-cookbook/ch32v003programmer/esp-idf/tinyusb/cmake_install.cmake")
endif()

