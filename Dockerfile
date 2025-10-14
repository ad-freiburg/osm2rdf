FROM ubuntu:24.04

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y git g++ libexpat1-dev cmake libbz2-dev libomp-dev
COPY . /app/
RUN cd /app/ && mkdir -p build && cd build && cmake .. && make -j
ENTRYPOINT ["/app/build/apps/osm2rdf"]
