all: matrixtest recttest cubetest polytest textest texcubetest screentest

matrixtest: matrix.cpp matrix.h matrixtest.cpp
	g++ -O3 -o matrixtest matrix.cpp matrixtest.cpp

recttest: matrix.cpp matrix.h raster.cpp raster.h recttest.cpp
	g++ -O3 -o recttest matrix.cpp raster.cpp recttest.cpp

cubetest: matrix.cpp matrix.h raster.cpp raster.h cubetest.cpp
	g++ -O3 -o cubetest matrix.cpp raster.cpp cubetest.cpp

polytest: matrix.cpp matrix.h raster.cpp raster.h polytest.cpp
	g++ -O3 -o polytest matrix.cpp raster.cpp polytest.cpp

textest: matrix.cpp matrix.h raster.cpp raster.h textest.cpp
	g++ -O3 -o textest matrix.cpp raster.cpp textest.cpp

texcubetest: matrix.cpp matrix.h raster.cpp raster.h texcubetest.cpp
	g++ -O3 -o texcubetest matrix.cpp raster.cpp texcubetest.cpp

screentest: matrix.cpp matrix.h raster.cpp raster.h screentest.cpp
	g++ -O3 -o screentest matrix.cpp raster.cpp screentest.cpp

.PHONY: clean
clean:
	rm -rf matrixtest
	rm -rf recttest
	rm -rf cubetest
	rm -rf polytest
	rm -rf textest
	rm -rf texcubetest
	rm -rf screentest
