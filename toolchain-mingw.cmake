# MinGW Toolchain File for MuseScore 스
# Configure CMAKE for MinGW compilation, disabling MSVC-specific flags

# Set system and processor
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Set compilers
set(CMAKE_C_COMPILER C:/Qt/Tools/mingw1310_64/bin/gcc.exe)
set(CMAKE_CXX_COMPILER C:/Qt/Tools/mingw1310_64/bin/g++.exe)
set(CMAKE_RC_COMPILER C:/Qt/Tools/mingw1310_64/bin/windres.exe)
set(CMAKE_MAKE_PROGRAM C:/Qt/Tools/mingw1310_64/bin/mingw32-make.exe)

# Disable MSVC mode - CRITICAL for preventing /wd flags
set(MSVC OFF CACHE BOOL "Not MSVC")
set(MSVC_IDE OFF CACHE BOOL "Not MSVC IDE")

# Add warning flags compatible with GCC
set(CMAKE_CXX_FLAGS_INIT "-Wno-unused-but-set-variable")
set(CMAKE_C_FLAGS_INIT "-Wno-unused-but-set-variable")


