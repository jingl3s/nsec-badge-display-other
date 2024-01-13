idf_component_register(SRCS "console.cpp" "cmd_neopixel.cpp"
                       INCLUDE_DIRS "."
                       REQUIRES console driver fatfs cmd_nvs cmd_system buzzer save neopixel
)
