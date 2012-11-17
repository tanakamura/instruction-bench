bench:
	cl /EHsc /MT /Zi /Ixbyak /Fa /O2 bench.cpp /link /DYNAMICBASE:NO
clean:
	del *~ bench.exe test.exe *.obj *.pdb *.ilk *.suo *.bin