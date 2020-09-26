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

perf: build
	cmake --build build --target osm2ttl
	for FILE in $(shell ls -Sr input); do echo $${FILE}; time perf record ./build/apps/osm2ttl "./input/$${FILE}" -o "/tmp/$${FILE}.qlever" --no-dump --use-ram-for-locations; perf report; done

docker:
	mkdir input || true
	mkdir output || true
	wget -O ./input/freiburg-regbez-latest.osm.pbf https://download.geofabrik.de/europe/germany/baden-wuerttemberg/freiburg-regbez-latest.osm.pbf
	wharfer build -t osm2ttl .
	wharfer run --rm -v `pwd`/input/:/input/ -v `pwd`/output/:/output/ -it osm2ttl /input/freiburg-regbez-latest.osm.pbf -o /output/freiburg-regbez-latest.osm.ttl
