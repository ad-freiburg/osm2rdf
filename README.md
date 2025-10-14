# osm2rdf

`osm2rdf` is a tool for converting the complete [OpenStreetMap](https://www.openstreetmap.org) (OSM) data to [RDF Turtle](https://www.w3.org/TR/turtle) (TTL).

It also pre-computes the spatial relations `sfIntersects`, `sfContains`, `sfCovers`, `sfTouches`, `sfCrosses`, `sfOverlaps`, and `sfEquals` between all OSM objects.

## Services and materials

Weekly updated downloads for the complete OSM as well as for individual countries are provided [here](https://osm2rdf.cs.uni-freiburg.de).

A SPARQL endpoint (based on [the QLever SPARQL engine](https://github.com/ad-freiburg/qlever)) for the complete OSM data is available under https://qlever.dev/osm-planet.
Example: [all buildings under a powerline][https://qlever.dev/osm-planet/Wd07W1].
The UI provides more example queries (click "Examples").

The basic method behind `osm2rdf` is described in [this SIGSPATIAL'21 paper](https://ad-publications.cs.uni-freiburg.de/SIGSPATIAL_osm2rdf_BBKL_2021.pdf).

The method behind computing the spatial relations is described in [this SIGSPATIAL'25 paper](https://ad-publications.cs.uni-freiburg.de/SIGSPATIAL_spatialjoin_BBK_2025.pdf).

## Requirements

  * cmake
  * Compiler for C++17
  * libexpat1
  * libbz2
  * libomp (optional, for multithreading)

## Quickstart

Fetch this repo:

```
$ git clone https://github.com/ad-freiburg/osm2rdf.git
```
Build `osm2rdf`:
```
$ cd osm2rdf
$ mkdir -p build && cd build
$ cmake ..
$ make -j
```
Fetch an OSM input file and print TTL to stdout:
```
$ wget https://download.geofabrik.de/europe/germany/baden-wuerttemberg/freiburg-regbez-latest.osm.pbf
$ ./apps/osm2rdf freiburg-regbex-latest.osm.pbf
```

For more options, see `--help`, `-h`, `-hh`, and `-hhh`.


### Docker

Alternatively you may use the provided `Dockerfile` to compile and run `osm2rdf`:
```
$ docker build -t osm2rdf .
```

```
$ mkdir input
$ wget -O input/freiburg-regbez-latest.osm.pbf https://download.geofabrik.de/europe/germany/baden-wuerttemberg/freiburg-regbez-latest.osm.pbf
$ docker run -v `pwd`/input/:/input/ -it osm2rdf /input/freiburg-regbex-latest.osm.pbf
```
