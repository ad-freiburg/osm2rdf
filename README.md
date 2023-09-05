# osm2rdf

`osm2rdf` is a tool for converting the complete [OpenStreetMap](https://www.openstreetmap.org) (OSM) data to [RDF Turtle](https://www.w3.org/TR/turtle) (TTL). The produced triples include:

1. One triple `<o> <key> <value>` for each key-value pair (called "tag" in OSM) of each object.
2. One triple `<o> geo:hasGeometry <wkt>` for the shape of each object (using [WKT](https://en.wikipedia.org/wiki/Well-known_text_representation_of_geometry) to represent the shape).
3. One triple `<o1> geo:sfContains <o2>` for each pair of objects, where `<o1>` geometrically contains `<o2>`.
4. One triple `<o1> geo:sfContains <o2>` for each pair of objects, where `<o1>` geometrically intersects `<o2>`.
5. Triples for various other kinds of information (see `osm2rdf --help` for the many options of the tool).

For the complete OSM data, the tool takes around 15 hours on standard hardware and produces around 40 billion triples with a size of around 200 GB for the compressed TTL file. 

## Accompanying services and materials

Weekly updated downloads for the complete data as well as for each individual country are provided on https://osm2rdf.cs.uni-freiburg.de .

A SPARQL endpoint (based on [the QLever SPARQL engine](https://github.com/ad-freiburg/qlever)) for the complete OSM data is available under https://qlever.cs.uni-freiburg.de/osm-planet . The UI provides many example queries (click on the "Examples" button).

The basic method behind `osm2rdf` is described in [this SIGSPATIAL'21 paper](https://ad-publications.cs.uni-freiburg.de/SIGSPATIAL_osm2rdf_BBKL_2021.pdf).

The algorithm behind computing the predicates `geo:sfContains` and `geo:sfIntersects` will be published soon. For the complete OSM data, there are around 2 billion non-trivial geometric objects (this count excludes points that are merely base points of more complex shapes). It is very challenging to compute geometric relations between such a large number of objects in finite time.

## Getting started

Some features require [boost](https://www.boost.org/) version **1.78 or newer**.
To enable these features boost has to be updated manually on most systems as distributions do ship older versions.

### Docker

The provided `Makefile` contains targets which build a Docker image, prepare input and output directories and downloads data.
To run `osm2rdf` on germany it is enough to call:
```
make docker-de
```
This creates the directories `input`, `output`, and `scratch`, downloads the data and runs `osm2rdf` with default parameters.

Alternatively use the provided `Dockerfile` to compile and run `osm2rdf`:
```
$ mkdir input
$ mkdir output
$ mkdir scratch

$ docker build -t osm2rdf .

$ wget -O ./input/germany-latest.osm.pbf https://download.geofabrik.de/europe/germany-latest.osm.pbf
$ docker run --rm -v `pwd`/input/:/input/ -v `pwd`/output/:/output/ -v `pwd`/scratch/:/scratch/ -it osm2rdf /input/germany-latest.osm.pbf -o /output/germany-latest.osm.ttl -t /scratch/
```

### Without Docker

On Ubuntu 20.04 the following packages are required to build osm2rdf:
```
clang clang-tidy g++ libboost-dev libboost-serialization-dev libexpat1-dev cmake libbz2-dev zlib1g-dev libomp-dev
```
`clang` is optional, but without it `clang-tidy` has [unrelated error messages](https://stackoverflow.com/a/52728225).

Clone and build `osm2rdf`:
```
$ git clone https://github.com/ad-freiburg/osm2rdf.git
$ cd osm2rdf
$ mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Debug .. && make -j4 && cd ..
```
Prepare `input`, `output` and `scratch` folders and download some data and convert the data to `ttl`:
```
$ mkdir input
$ mkdir output
$ mkdir scratch
$ wget -O ./input/freiburg-regbez-latest.osm.pbf https://download.geofabrik.de/europe/germany/baden-wuerttemberg/freiburg-regbez-latest.osm.pbf
$ ./build/osm2rdf ./input/freiburg-regbez-latest.osm.pbf -o ./output/freiburg-regbez-latest.osm.ttl
```

More `.pbf` files can be found on the [Geofabrik Download Server](https://download.geofabrik.de/index.html)
