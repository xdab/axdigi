.PHONY: build release run test clean install update

.DEFAULT_GOAL := run

build:
	mkdir -p build
	cd build && cmake -DCMAKE_BUILD_TYPE=Debug -G "Unix Makefiles" ..
	cd build && make -j

release:
	mkdir -p build
	cd build && cmake -G "Unix Makefiles" ..
	cd build && make -j

run: build
	./build/axdigi

test: build
	./build/ax_test

clean:
	rm -rf build

install: release
	sudo make -C build install

update:
	git pull
	git submodule update --init --recursive
