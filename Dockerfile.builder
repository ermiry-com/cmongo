FROM gcc as builder

WORKDIR /opt

RUN apt-get update && apt-get install -y cmake libssl-dev

# mongo c driver
ARG MONGOC_VERSION=1.17.3

RUN mkdir /opt/mongoc && cd /opt/mongoc \
    && wget -q https://github.com/mongodb/mongo-c-driver/releases/download/${MONGOC_VERSION}/mongo-c-driver-${MONGOC_VERSION}.tar.gz \
    && tar xzf mongo-c-driver-${MONGOC_VERSION}.tar.gz \
    && cd mongo-c-driver-${MONGOC_VERSION} \
    && mkdir cmake-build && cd cmake-build \
    && cmake -D ENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF -D CMAKE_BUILD_TYPE=Release -D ENABLE_SSL=OPENSSL -D ENABLE_SRV=ON .. \
    && make -j4 && make install

# cmongo
WORKDIR /opt/cmongo
COPY . .
RUN make TYPE=production -j4

############
FROM gcc

ARG RUNTIME_DEPS='ca-certificates wget libssl-dev'

RUN apt-get update && apt-get install -y ${RUNTIME_DEPS} && apt-get clean

# mongoc files
COPY --from=builder /usr/lib/x86_64-linux-gnu/libicudata.so.63 /usr/lib/x86_64-linux-gnu/libicudata.so.63
COPY --from=builder /usr/lib/x86_64-linux-gnu/libicuuc.so.63 /usr/lib/x86_64-linux-gnu/libicuuc.so.63
COPY --from=builder /usr/local/include/ /usr/local/include/
COPY --from=builder /usr/local/lib/libmongoc-1.0.so /usr/local/lib/libmongoc-1.0.so
COPY --from=builder /usr/local/lib/libbson-1.0.so /usr/local/lib/libbson-1.0.so

# cmongo
COPY --from=builder /opt/cmongo/bin/libcmongo.so /usr/local/lib/
COPY --from=builder /opt/cmongo/include/cmongo /usr/local/include/cmongo

RUN ldconfig

CMD ["/bin/bash"]
