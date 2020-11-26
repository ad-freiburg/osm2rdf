all: compile checkstyle test benchmark

clean:
	rm -rf build

build:
	cmake -S . -B build

compile: build
	cmake --build build --target osm2ttl osm2ttl-maxid

checkstyle:

test: build
	cmake --build build --target run_tests

benchmark: build
	cmake --build build --target run_benchmarks

run: compile
	for FILE in $(shell ls -Sr input); do ./build/apps/osm2ttl "./input/$${FILE}" -o "/tmp/$${FILE}.qlever" --write-statistics; done

perf: compile
	for FILE in $(shell ls -Sr input); do time perf record ./build/apps/osm2ttl "./input/$${FILE}" -o "/tmp/$${FILE}.qlever"; perf report; done

perf-st: compile
	for FILE in $(shell ls -Sr input); do export OMP_NUM_THREADS=1 && export OMP_THREAD_LIMIT=1 && time perf record ./build/apps/osm2ttl "./input/$${FILE}" -o "/tmp/$${FILE}.qlever"; perf report; done

docker-fr:
	mkdir input || true
	mkdir output || true
	wharfer build -t osm2ttl .
	wharfer run --rm -v `pwd`/input/:/input/ -v `pwd`/output/:/output/ -it osm2ttl /input/freiburg-regbez-latest.osm.pbf -o /output/freiburg-regbez-latest.osm.ttl

docker-bw:
	mkdir input || true
	mkdir output || true
	wharfer build -t osm2ttl .
	wharfer run --rm -v `pwd`/input/:/input/ -v `pwd`/output/:/output/ -it osm2ttl /input/baden-wuerttemberg-latest.osm.pbf -o /output/baden-wuerttemberg-latest.osm.ttl

docker-de:
	mkdir input || true
	mkdir output || true
	wharfer build -t osm2ttl .
	wharfer run --rm -v `pwd`/input/:/input/ -v `pwd`/output/:/output/ -it osm2ttl /input/germany-latest.osm.pbf -o /output/germany-latest.osm.ttl
