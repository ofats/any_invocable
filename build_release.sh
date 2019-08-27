cd `dirname ${BASH_SOURCE[0]}`
mkdir -p build/Release
cd build/Release
cmake ../.. -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
make -j
