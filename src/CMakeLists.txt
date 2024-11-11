# Add library c files
add_library(piconfc piconfc.c piconfc_PN532.c piconfc_I2C.c piconfc_NTAG.c piconfc_NDEF.c)

# Add include directory
target_include_directories(piconfc PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../include)

# Add the standard library to the build
target_link_libraries(piconfc PUBLIC pico_stdlib hardware_i2c pico_stdio)

#Uncomment for debugging
# target_compile_definitions(piconfc PRIVATE DEBUG=1)