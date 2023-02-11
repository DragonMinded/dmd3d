all: matrixtest recttest cubetest polytest textest texcubetest screentest stltest

# Engine stuff first.
matrix.o: matrix.cpp matrix.h
	g++ -O3 -g -c -o matrix.o matrix.cpp

raster.o: raster.cpp raster.h
	g++ -O3 -g -c -o raster.o raster.cpp

model.o: model.cpp model.h
	g++ -O3 -g -c -o model.o model.cpp

# Test executables.
matrixtest: matrix.o matrixtest.cpp
	g++ -O3 -g -o matrixtest matrix.o matrixtest.cpp

recttest: matrix.o raster.o recttest.cpp
	g++ -O3 -g -o recttest matrix.o raster.o recttest.cpp

cubetest: matrix.o raster.o cubetest.cpp
	g++ -O3 -g -o cubetest matrix.o raster.o cubetest.cpp

polytest: matrix.o raster.o polytest.cpp
	g++ -O3 -g -o polytest matrix.o raster.o polytest.cpp

textest: matrix.o raster.o textest.cpp
	g++ -O3 -g -o textest matrix.o raster.o textest.cpp

texcubetest: matrix.o raster.o texcubetest.cpp
	g++ -O3 -g -o texcubetest matrix.o raster.o texcubetest.cpp

screentest: matrix.o raster.o screentest.cpp
	g++ -O3 -g -o screentest matrix.o raster.o screentest.cpp

stltest: matrix.o raster.o model.o stltest.cpp
	g++ -O3 -g -o stltest matrix.o raster.o model.o stltest.cpp

.PHONY: clean
clean:
	rm -rf matrixtest
	rm -rf recttest
	rm -rf cubetest
	rm -rf polytest
	rm -rf textest
	rm -rf texcubetest
	rm -rf screentest
	rm -rf stltest
