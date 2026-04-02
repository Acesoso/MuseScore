#!/bin/bash
# MinGW C++ compiler wrapper - removes MSVC-specific flags

# Get the real compiler path
REAL_CXX="C:/Qt/Tools/mingw1310_64/bin/g++.exe"

# Filter out /wd flags and rebuild arguments
declare -a ARGS
for arg in "$@"; do
    # Skip /wd flags (MSVC warning suppressions)
    if [[ "$arg" != /wd* ]]; then
        ARGS+=("$arg")
    fi
done

# Call the real compiler with filtered arguments
"$REAL_CXX" "${ARGS[@]}"
