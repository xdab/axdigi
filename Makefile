.PHONY: build run clean install update

.DEFAULT_GOAL := run

build:
	mkdir -p build
	cd build && cmake .. && make -j

run: build
	./build/axdigi

clean:
	rm -rf build

install: build
	cd build && make install

update:
	git pull
	git submodule update --init --recursive
