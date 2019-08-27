cd `dirname ${BASH_SOURCE[0]}`
mkdir -p build/Debug
cd build/Debug
cmake ../.. -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
make -j
cd ../../
ln -fs build/Debug/compile_commands.json compile_commands.json
