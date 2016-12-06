all: bench

bench-mpx: bench.cpp
	-g++ -DENABLE_MPX -DXBYAK_NO_OP_NAMES -std=c++0x -O2 -Ixbyak/xbyak -o bench bench.cpp -mmpx -fcheck-pointer-bounds -static

bench: bench.cpp
	-cl /EHsc /MT /Zi /Ixbyak/xbyak /Fa /O2 bench.cpp /link /DYNAMICBASE:NO
	-g++ -DXBYAK_USE_MMAP_ALLOCATOR -DXBYAK_NO_OP_NAMES -std=c++0x -O2 -Ixbyak/xbyak -o bench bench.cpp

clean:
	-del *~ bench.exe test.exe *.obj *.pdb *.ilk *.suo *.bin *.o bench
	-rm -f *~ bench.exe test.exe *.obj *.pdb *.ilk *.suo *.bin *.o bench
