# cmake_minimum_required(VERSION 3.13)

# # Pull in Pico SDK (must be before project)
# set(PICO_SDK_PATH /usr/share/pico-sdk)
# # set(picotool_FOUND 1)
# # set(Env{PICOTOOL_FETCH_FROM_GIT_PATH} /usr/share/picotool)
# set(Env{PICO_SDK_PATH} /usr/share/pico-sdk)



# include(pico_sdk_import.cmake)
# set(PICO_BOARD "waveshare_rp2040_zero")
# project(PicoNFC)

# # Initialise the Pico SDK
# pico_sdk_init()

add_subdirectory(./src)

# add_subdirectory(./tests)




# file(GLOB SRC ${CMAKE_CURRENT_LIST_DIR}/*.c ${CMAKE_CURRENT_LIST_DIR}/*.h)

add_executable(testing-1 testing.c)
target_link_libraries(testing-1 PUBLIC pico_stdio pico_stdlib piconfc pico_time)
pico_enable_stdio_usb(testing-1 1)
pico_add_extra_outputs(testing-1)