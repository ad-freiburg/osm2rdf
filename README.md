# osm2ttl
Convert OpenStreetMap (OSM) data to Turtle (TTL). `osm2ttl` converts various formats using [Libosmium](https://github.com/osmcode/libosmium) into valid rdf files.

## Getting started

### Docker

The provided `Makefile` contains targets which build a Docker image, prepare input and output directories and downloads data.
To run `osm2ttl` on germany it is enough to call:
```
make docker-de
```
This creates the directories `input`, `output`, and `scratch`, downloads the data and runs `osm2ttl` with default parameters.

Alternatively use the provided `Dockerfile` to compile and run `osm2ttl`:
```
$ mkdir input
$ mkdir output
$ mkdir scratch

$ docker build -t osm2ttl .

$ wget -O ./input/germany-latest.osm.pbf https://download.geofabrik.de/europe/germany-latest.osm.pbf
$ docker run --rm -v `pwd`/input/:/input/ -v `pwd`/output/:/output/ -v `pwd`/scratch/:/scratch/ -it osm2ttl /input/germany-latest.osm.pbf -o /output/germany-latest.osm.ttl -t /scratch/
```

### Without Docker

On Ubuntu 20.04 the following packages are required to build osm2ttl:
```
clang clang-tidy g++ libboost-dev libexpat1-dev cmake libbz2-dev zlib1g-dev libomp-dev
```
`clang` is optional, but without it `clang-tidy` has [unrelated error messages](https://stackoverflow.com/a/52728225).

Clone and build `osm2ttl`:
```
$ git clone https://github.com/ad-freiburg/osm2ttl.git
$ cd osm2ttl
$ mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Debug .. && make -j4 && cd ..
```
Prepare `input`, `output` and `scratch` folders and download some data and convert the data to `ttl`:
```
$ mkdir input
$ mkdir output
$ mkdir scratch
$ wget -O ./input/freiburg-regbez-latest.osm.pbf https://download.geofabrik.de/europe/germany/baden-wuerttemberg/freiburg-regbez-latest.osm.pbf
$ ./build/osm2ttl ./input/freiburg-regbez-latest.osm.pbf -o ./output/freiburg-regbez-latest.osm.ttl
```

More `.pbf` files can be found on the [Geofrabik Download Server](https://download.geofabrik.de/index.html)
