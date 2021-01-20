#!/bin/bash

TARGET=$(pwd)/novelsm

# Need to replace "rocksdb/" - > "leveldb/"
# Need to replace "rocksdb::" -> "leveldb::"
for FILE in $TARGET/*.cpp $TARGET/*.hpp; do
    sed -i -e 's/rocksdb\//leveldb\//g' $FILE
    sed -i -e 's/rocksdb::/leveldb::/g' $FILE
done
