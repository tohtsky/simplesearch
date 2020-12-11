FROM python:3.7.9-slim-stretch

WORKDIR /work/

RUN apt-get -y update && apt-get install -y --no-install-recommends \
    ca-certificates \
    g++ \
    git \
    curl \
    sudo \
    make \
    xz-utils \
    patch \
    file \
    perl \
    cmake \
    wget \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*  

RUN mkdir /work/tmp && \
    wget https://github.com/linux-test-project/lcov/releases/download/v1.12/lcov-1.12.tar.gz -O /work/tmp/lcov-1.12.tar.gz && \
    tar xfz /work/tmp/lcov-1.12.tar.gz -C /work/tmp/ && \
    cd /work/tmp/lcov-1.12 && \
    make install
COPY include /work/include/
COPY src /work/src
COPY tests /work/tests
COPY CMakeLists.txt /work/CMakeLists.txt
RUN mkdir build && cd build && cmake .. && \
    make -j8 && ctest && echo `ls` 
RUN cd build/CMakeFiles/unit_tests.dir && \
    echo `ls` && \
    lcov -d `pwd` -c -o coverage.info && \
    lcov -r coverage.info catch.hpp */c++/* */cpp-btree/* */nlohmann/* */tests/* -o coverageFiltered.info && \
    genhtml -o /work/generated coverageFiltered.info

WORKDIR /work/generated/
CMD ["python",  "-m", "http.server"]
