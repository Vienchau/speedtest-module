#!/bin/sh

make clean
make cross=1
curl --upload-file ./speedtest_module http://docker.hieunv.us:12345/speedtest_module
