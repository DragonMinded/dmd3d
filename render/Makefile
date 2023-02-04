all: matrixtest recttest cubetest polytest

matrixtest: matrix.cpp matrix.h matrixtest.cpp
	g++ -O3 -o matrixtest matrix.cpp matrixtest.cpp

recttest: matrix.cpp matrix.h raster.cpp raster.h recttest.cpp
	g++ -O3 -o recttest matrix.cpp raster.cpp recttest.cpp

cubetest: matrix.cpp matrix.h raster.cpp raster.h cubetest.cpp
	g++ -O3 -o cubetest matrix.cpp raster.cpp cubetest.cpp

polytest: matrix.cpp matrix.h raster.cpp raster.h polytest.cpp
	g++ -O3 -o polytest matrix.cpp raster.cpp polytest.cpp

.PHONY: clean
clean:
	rm -rf matrixtest
	rm -rf recttest
	rm -rf cubetest
	rm -rf polytest
