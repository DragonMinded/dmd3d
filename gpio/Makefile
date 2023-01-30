all: driver

driver: driver.c
	gcc -O3 -o driver driver.c libwiringx.a

.PHONY: clean
clean:
	rm -rf driver
