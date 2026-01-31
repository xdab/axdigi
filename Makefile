.PHONY: build run test clean install update

.DEFAULT_GOAL := run

build:
	mkdir -p build
	cd build && cmake .. && make -j

run: build
	./build/axdigi

test: build
	./build/ax_test

clean:
	rm -rf build

install: build
	cd build && make install

update:
	git pull
	git submodule update --init --recursive
