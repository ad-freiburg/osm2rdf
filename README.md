# osm2ttl
Convert OpenStreetMap (OSM) data to Turtle (TTL)

## Getting started

On Ubuntu 20.04 the following packages are required to build osm2ttl:
```
clang clang-tidy g++ libboost-dev libexpat1-dev cmake libbz2-dev zlib1g-dev libomp-dev
```
`clang` is optional, but without it `clang-tidy` has [unrelated error messages](https://stackoverflow.com/a/52728225).
`libomp-dev` is optional, but enables multithreading for the calculation of the contains relation

Clone and build `osm2ttl`:
```
$ git clone git@github.com:ad-freiburg/osm2ttl.git
$ cd osm2ttl
$ mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Debug .. && make -j4 && cd ..
```
Prepare input and output folders and download some data and convert the data to `ttl`:
```
$ mkdir input
$ mkdir output
$ wget -O ./input/freiburg-regbez-latest.osm.pbf https://download.geofabrik.de/europe/germany/baden-wuerttemberg/freiburg-regbez-latest.osm.pbf
$ ./build/osm2ttl ./input/freiburg-regbez-latest.osm.pbf -o ./output/freiburg-regbez-latest.osm.ttl
```

Alternativly use the provided `Dockerfile` to compile and run `osm2ttl`:
```
$ docker build -t osm2ttl .
$ docker run --rm -v `pwd`/input/:/input/ -v `pwd`/output/:/output/ -it osm2ttl /input/freiburg-regbez-latest.osm.pbf -o /output/freiburg-regbez-latest.osm.ttl

$ wget -O ./input/baden-wuerttemberg-latest.osm.pbf https://download.geofabrik.de/europe/germany/baden-wuerttemberg-latest.osm.pbf
$ docker run --rm -v `pwd`/input/:/input/ -v `pwd`/output/:/output/ -it osm2ttl /input/baden-wuerttemberg-latest.osm.pbf -o /output/baden-wuerttemberg-latest.osm.ttl
$ wget -O ./input/germany-latest.osm.pbf https://download.geofabrik.de/europe/germany-latest.osm.pbf
$ docker run --rm -v `pwd`/input/:/input/ -v `pwd`/output/:/output/ -it osm2ttl /input/germany-latest.osm.pbf -o /output/germany-latest.osm.ttl
$ wget -O ./input/europe-latest.osm.pbf https://download.geofabrik.de/europe-latest.osm.pbf
$ docker run --rm -v `pwd`/input/:/input/ -v `pwd`/output/:/output/ -it osm2ttl /input/europe-latest.osm.pbf -o /output/europe-latest.osm.ttl
$ wget -O ./input/planet-latest.osm.pbf https://planet.openstreetmap.org/pbf/planet-latest.osm.pbf
$ docker run --rm -v `pwd`/input/:/input/ -v `pwd`/output/:/output/ -it osm2ttl /input/planet-latest.osm.pbf -o /output/planet-latest.osm.ttl
```

More `.pbf` files can be found on the [Geofrabik Download Server](https://download.geofabrik.de/index.html)
