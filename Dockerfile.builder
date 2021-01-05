FROM ermiry/mongoc:builder as builder

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
