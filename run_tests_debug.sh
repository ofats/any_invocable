cd `dirname ${BASH_SOURCE[0]}`
[ ! -d "./build" ] && echo "Error: build project first using ./build_debug.sh" && exit
cd build/Debug
ctest $*
