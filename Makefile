all: compile checkstyle test benchmark

clean:
	rm -rf *build*

build:
	cmake -S . -B build

compile: build
	cmake --build build

checkstyle:

test: build
	cmake --build build --target test

benchmark: build
	cmake --build build --target benchmarks
