cd ./cmake-build-debug
./cool
cd ..
llc -filetype obj ./cmake-build-debug/output.ll -o output.o
gcc -c runtime/runtime.h runtime/runtime.c
gcc -o exe output.o runtime.o
rm runtime.o output.o
./exe
