all: compile checkstyle test benchmark

clean:
	rm -rf build

build:
	cmake -S . -B build

compile: build
	cmake --build build --target osm2ttl osm2ttl-stats

checkstyle:

test: build
	cmake --build build --target run_tests

benchmark: build
	cmake --build build --target run_benchmarks

run: compile
	for FILE in $(shell ls -Sr input); do ./build/apps/osm2ttl "./input/$${FILE}" -o "/tmp/$${FILE}.qlever"; done

perf: compile
	for FILE in $(shell ls -Sr input); do time perf record ./build/apps/osm2ttl "./input/$${FILE}" -o "/tmp/$${FILE}.qlever"; perf report; done

perf-st: compile
	for FILE in $(shell ls -Sr input); do export OMP_NUM_THREADS=1 && export OMP_THREAD_LIMIT=1 && time perf record ./build/apps/osm2ttl "./input/$${FILE}" -o "/tmp/$${FILE}.qlever"; perf report; done

docker-fr:
	mkdir input || true
	mkdir output || true
	wharfer build -t osm2ttl .
	wharfer run --rm -v `pwd`/input/:/input/ -v `pwd`/output/:/output/ -v `pwd`/scratch/:/scratch/ -it osm2ttl /input/freiburg-regbez-latest.osm.pbf -o /output/freiburg-regbez-latest.osm.ttl -t /scratch/ --add-inverse-relation-direction

docker-bw:
	mkdir input || true
	mkdir output || true
	wharfer build -t osm2ttl .
	wharfer run --rm -v `pwd`/input/:/input/ -v `pwd`/output/:/output/ -v `pwd`/scratch/:/scratch/ -it osm2ttl /input/baden-wuerttemberg-latest.osm.pbf -o /output/baden-wuerttemberg-latest.osm.ttl -t /scratch/ --add-inverse-relation-direction

docker-de:
	mkdir input || true
	mkdir output || true
	wharfer build -t osm2ttl .
	wharfer run --rm -v `pwd`/input/:/input/ -v `pwd`/output/:/output/ -v `pwd`/scratch/:/scratch/ -it osm2ttl /input/germany-latest.osm.pbf -o /output/germany-latest.osm.ttl -t /scratch/ --add-inverse-relation-direction

docker-eu:
	mkdir input || true
	mkdir output || true
	wharfer build -t osm2ttl .
	wharfer run --rm -v `pwd`/input/:/input/ -v `pwd`/output/:/output/ -v `pwd`/scratch/:scratch/ -it osm2ttl /input/europe-latest.osm.pbf -o /output/europe-latest.osm.ttl -t /scratch/ --add-inverse-relation-direction --store-locations-on-disk
