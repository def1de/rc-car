cmake_minimum_required(VERSION 3.10)

# Define the project name
project(Blink C CXX)

# Define the target MCU and clock frequency
set(MCU atmega32u4)
set(F_CPU 16000000UL)

# Define the source file
set(SRC blink.cpp)

# Define the compiler and flags
set(CMAKE_C_COMPILER avr-gcc)
set(CMAKE_CXX_COMPILER avr-g++)
set(CMAKE_C_FLAGS "-mmcu=${MCU} -DF_CPU=${F_CPU} -Os")
set(CMAKE_CXX_FLAGS "-mmcu=${MCU} -DF_CPU=${F_CPU} -Os")

# Add the executable
add_executable(blink ${SRC})

# Define the output files
set(ELF ${CMAKE_BINARY_DIR}/blink)
set(HEX ${CMAKE_BINARY_DIR}/blink.hex)

# Add a custom command to convert the ELF file to a HEX file
add_custom_command(TARGET blink POST_BUILD
    COMMAND avr-objcopy -O ihex -R .eeprom ${ELF} ${HEX}
    COMMENT "Converting ELF to HEX"
)

# Add a custom target to upload the HEX file to the microcontroller
add_custom_target(upload
    COMMAND sudo avrdude -v -p ${MCU} -c avr109 -P /dev/ttyACM0 -b 57600 -U flash:w:${HEX}:i
    DEPENDS ${HEX}
    COMMENT "Uploading HEX to microcontroller"
)

# Add a custom target to clean up generated files
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E remove ${ELF} ${HEX}
    COMMENT "Cleaning up generated files"
)