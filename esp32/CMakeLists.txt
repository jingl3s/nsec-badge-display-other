# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS "./extra-components/")
#add_compile_definitions(SDCARD_ENABLED=0)
list(APPEND EXTRA_COMPONENT_DIRS extra-components/lvgl_esp32_drivers extra-components/lvgl_esp32_drivers/lvgl_touch extra-components/lvgl_esp32_drivers/lvgl_tft)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(nsec-badge-controller-screen)
