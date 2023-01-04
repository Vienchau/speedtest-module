#!/bin/sh

make clean
make
curl --upload-file ./speedtest_module http://docker.hieunv.us:12345/speedtest_module
