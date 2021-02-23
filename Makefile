all: compile checkstyle test benchmark

clean:
	rm -rf build

clean-input:
	rm -rf ./input/

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

download-all: input/freiburg-regbez-latest.osm.pbf input/baden-wuerttemberg-latest.osm.pbf input/germany-latest.osm.pbf input/europe-latest.osm.pbf input/planet-latest.osm.pbf

input:
	mkdir input || true

input/freiburg-regbez-latest.osm.pbf: input
	wget -O ./input/freiburg-regbez-latest.osm.pbf https://download.geofabrik.de/europe/germany/baden-wuerttemberg/freiburg-regbez-latest.osm.pbf

input/baden-wuerttemberg-latest.osm.pbf: input
	wget -O ./input/baden-wuerttemberg-latest.osm.pbf https://download.geofabrik.de/europe/germany/baden-wuerttemberg-latest.osm.pbf

input/germany-latest.osm.pbf: input
	wget -O ./input/germany-latest.osm.pbf https://download.geofabrik.de/europe/germany-latest.osm.pbf

input/europe-latest.osm.pbf: input
	wget -O ./input/europe-latest.osm.pbf https://download.geofabrik.de/europe-latest.osm.pbf

input/planet-latest.osm.pbf: input
	wget -O ./input/planet-latest.osm.pbf https://planet.openstreetmap.org/pbf/planet-latest.osm.pbf

docker-dirs:
	mkdir output || true
	chmod 777 output
	mkdir scratch || true
	chmod 777 scratch

docker-fr: docker-dirs input/freiburg-regbez-latest.osm.pbf
	wharfer build -t osm2ttl .
	wharfer run --rm -v `pwd`/input/:/input/ -v `pwd`/output/:/output/ -v `pwd`/scratch/:/scratch/ -it osm2ttl /input/freiburg-regbez-latest.osm.pbf -o /output/freiburg-regbez-latest.osm.ttl -t /scratch/ --add-inverse-relation-direction

docker-bw: docker-dirs input/baden-wuerttemberg.osm.pbf
	wharfer build -t osm2ttl .
	wharfer run --rm -v `pwd`/input/:/input/ -v `pwd`/output/:/output/ -v `pwd`/scratch/:/scratch/ -it osm2ttl /input/baden-wuerttemberg-latest.osm.pbf -o /output/baden-wuerttemberg-latest.osm.ttl -t /scratch/ --add-inverse-relation-direction

docker-de: docker-dirs input/germany-latest.osm.pbf
	wharfer build -t osm2ttl .
	wharfer run --rm -v `pwd`/input/:/input/ -v `pwd`/output/:/output/ -v `pwd`/scratch/:/scratch/ -it osm2ttl /input/germany-latest.osm.pbf -o /output/germany-latest.osm.ttl -t /scratch/ --add-inverse-relation-direction

docker-eu: docker-dirs input/europe-latest.osm.pbf
	wharfer build -t osm2ttl .
	wharfer run --rm -v `pwd`/input/:/input/ -v `pwd`/output/:/output/ -v `pwd`/scratch/:/scratch/ -it osm2ttl /input/europe-latest.osm.pbf -o /output/europe-latest.osm.ttl -t /scratch/ --add-inverse-relation-direction

docker-pl: docker-dirs input/planet-latest.osm.pbf
	wharfer build -t osm2ttl .
	wharfer run --rm -v `pwd`/input/:/input/ -v `pwd`/output/:/output/ -v `pwd`/scratch/:/scratch/ -it osm2ttl /input/planet-latest.osm.pbf -o /output/planet-latest.osm.ttl -t /scratch/ --add-inverse-relation-direction
