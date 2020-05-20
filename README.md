# jsonConverter
Converts json data to blob or msgpack

[![Apache V2 License](http://img.shields.io/badge/license-Apache%20V2-blue.svg)](https://github.com/gbuddappagari/jsonConverter/blob/master/LICENSE.txt)

# Building and Testing Instructions

```
mkdir build
cd build
cmake ..
make

cd src
./jsonConverterCli -f <file-name> --[encoding]
Encoding --B for blob
         --M for msgpack

cd multipart
./multipartDoc <root-version> <subdoc-version,subdoc-name,data> .... <subdoc-version,subdoc-name,data>
```
