all: bench

CXXFLAGS=-DXBYAK_USE_MMAP_ALLOCATOR -DXBYAK_NO_OP_NAMES -std=c++0x -O2 -Ixbyak/xbyak -MMD


bench: bench.o gen.o sse.o avx.o avx512.o
	#-cl /EHsc /MT /Zi /Ixbyak/xbyak /Fa /O2 bench.cpp /link /DYNAMICBASE:NO
	-g++ -o bench $^

clean:
	-del *~ bench.exe test.exe *.obj *.pdb *.ilk *.suo *.bin *.o bench
	-rm -f *~ bench.exe test.exe *.obj *.pdb *.ilk *.suo *.bin *.o bench

-include *.d