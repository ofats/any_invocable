cd `dirname ${BASH_SOURCE[0]}`

CC=gcc
CXX=g++
CXX_FLAGS="-g -fsanitize=address,leak,undefined -fno-sanitize-recover=undefined -Wall -Wextra -Werror"
LINKER_FLAGS="-lasan -lubsan -fuse-ld=gold"

mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="${CXX_FLAGS}" -DCMAKE_EXE_LINKER_FLAGS="${LINKER_FLAGS}" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
ln -fs build/compile_commands.json ../compile_commands.json
cmake --build . -j
ctest $*
