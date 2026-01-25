.PHONY: build run clean install

.DEFAULT_GOAL := run

build:
	mkdir -p build
	cd build && cmake .. && make

run: build
	./build/axdigi

clean:
	rm -rf build

install: build
	cd build && make install
