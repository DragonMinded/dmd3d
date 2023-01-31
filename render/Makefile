all: matrixtest cubetest

matrixtest: matrix.cpp matrix.h matrixtest.cpp
	g++ -O3 -o matrixtest matrix.cpp matrixtest.cpp

cubetest: matrix.cpp matrix.h cubetest.cpp
	g++ -O3 -o cubetest matrix.cpp cubetest.cpp

.PHONY: clean
clean:
	rm -rf matrixtest
