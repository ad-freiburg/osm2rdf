# Copyright 2020, University of Freiburg
# Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

# This file is part of osm2rdf.
#
# osm2rdf is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# osm2rdf is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with osm2rdf.  If not, see <https://www.gnu.org/licenses/>.

DOCKER:=$(shell which podman || which wharfer || which docker)

all: compile checkstyle test benchmark

clean:
	rm -rf build

clean-input:
	rm -rf ./input/

build:
	cmake -S . -B build

compile: build
	cmake --build build --target osm2rdf osm2rdf-stats

checkstyle:

test: build
	cmake --build build --target run_tests

benchmark: build
	cmake --build build --target run_benchmarks

run: compile
	for FILE in $(shell ls -Sr input); do ./build/apps/osm2rdf "./input/$${FILE}" -o "/tmp/$${FILE}.qlever"; done

perf: compile
	for FILE in $(shell ls -Sr input); do time perf record ./build/apps/osm2rdf "./input/$${FILE}" -o "/tmp/$${FILE}.qlever"; perf report; done

perf-st: compile
	for FILE in $(shell ls -Sr input); do export OMP_NUM_THREADS=1 && export OMP_THREAD_LIMIT=1 && time perf record ./build/apps/osm2rdf "./input/$${FILE}" -o "/tmp/$${FILE}.qlever"; perf report; done

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
	mkdir input || true
	chmod 777 input
	mkdir output || true
	chmod 777 output
	mkdir scratch || true
	chmod 777 scratch

docker-fr: docker-dirs input/freiburg-regbez-latest.osm.pbf
	${DOCKER} build -t osm2rdf .
	${DOCKER} run --rm -v `pwd`/input/:/input/ -v `pwd`/output/:/output/ -v `pwd`/scratch/:/scratch/ -it osm2rdf /input/freiburg-regbez-latest.osm.pbf -o /output/freiburg-regbez-latest.osm.ttl -t /scratch/

docker-bw: docker-dirs input/baden-wuerttemberg-latest.osm.pbf
	${DOCKER} build -t osm2rdf .
	${DOCKER} run --rm -v `pwd`/input/:/input/ -v `pwd`/output/:/output/ -v `pwd`/scratch/:/scratch/ -it osm2rdf /input/baden-wuerttemberg-latest.osm.pbf -o /output/baden-wuerttemberg-latest.osm.ttl -t /scratch/

docker-de: docker-dirs input/germany-latest.osm.pbf
	${DOCKER} build -t osm2rdf .
	${DOCKER} run --rm -v `pwd`/input/:/input/ -v `pwd`/output/:/output/ -v `pwd`/scratch/:/scratch/ -it osm2rdf /input/germany-latest.osm.pbf -o /output/germany-latest.osm.ttl -t /scratch/

docker-eu: docker-dirs input/europe-latest.osm.pbf
	${DOCKER} build -t osm2rdf .
	${DOCKER} run --rm -v `pwd`/input/:/input/ -v `pwd`/output/:/output/ -v `pwd`/scratch/:/scratch/ -it osm2rdf /input/europe-latest.osm.pbf -o /output/europe-latest.osm.ttl -t /scratch/

docker-pl: docker-dirs input/planet-latest.osm.pbf
	${DOCKER} build -t osm2rdf .
	${DOCKER} run --rm -v `pwd`/input/:/input/ -v `pwd`/output/:/output/ -v `pwd`/scratch/:/scratch/ -it osm2rdf /input/planet-latest.osm.pbf -o /output/planet-latest.osm.ttl -t /scratch/

docker-fr-ratios: docker-dirs input/freiburg-regbez-latest.osm.pbf
	${DOCKER} build -t osm2rdf .
	for R in "-1" "0.001" "0.01" "0.1" "0.2" "0.25" "0.5" "0.75" "0.9" "1.0"; do ${DOCKER} run --rm -v `pwd`/input/:/input/ -v `pwd`/output/:/output/ -v `pwd`/scratch/:/scratch/ -it osm2rdf /input/freiburg-regbez-latest.osm.pbf -o /output/freiburg-regbez-latest.osm.ttl -t /scratch/ --minimal-area-envelope-ratio $$R --no-node-geometric-relations --no-way-geometric-relations; done

docker-bw-ratios: docker-dirs input/baden-wuerttemberg-latest.osm.pbf
	${DOCKER} build -t osm2rdf .
	for R in "-1" "0.001" "0.01" "0.1" "0.2" "0.25" "0.5" "0.75" "0.9" "1.0"; do ${DOCKER} run --rm -v `pwd`/input/:/input/ -v `pwd`/output/:/output/ -v `pwd`/scratch/:/scratch/ -it osm2rdf /input/baden-wuerttemberg-latest.osm.pbf -o /output/baden-wuerttemberg-latest.ttl -t /scratch/ --minimal-area-envelope-ratio $$R --no-node-geometric-relations --no-way-geometric-relations; done
