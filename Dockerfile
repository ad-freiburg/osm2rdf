FROM ubuntu:18.04

RUN apt-get update && apt-get install -y clang clang-tidy g++ libboost-dev libexpat1-dev cmake libbz2-dev zlib1g-dev
COPY . /app/
RUN cd /app/ && rm -rf build && mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Debug .. && make -j
ENTRYPOINT ["/app/build/osm2ttl"]
