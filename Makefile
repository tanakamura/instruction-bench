bench: bench.cpp
	-cl /EHsc /MT /Zi /Ixbyak/xbyak /Fa /O2 bench.cpp /link /DYNAMICBASE:NO
	-g++ -DXBYAK_NO_OP_NAMES -std=c++0x -O2 -Ixbyak/xbyak -o bench bench.cpp -mmpx -fcheck-pointer-bounds -static

clean:
	-del *~ bench.exe test.exe *.obj *.pdb *.ilk *.suo *.bin *.o bench
	-rm -f *~ bench.exe test.exe *.obj *.pdb *.ilk *.suo *.bin *.o bench