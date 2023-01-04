## Speed test module using libcURL.

### Compile:

- `CROSS_COMPILE`: Toolchain's path if we use cross-compiling.

```bash
#cross-compile
make cross=1

#x86-64 compile
make
```

### Define:

- `TEST_SIZE`: The size of each thread to download/upload test.
- `TEST_COUNTER`: The number of threads.
- `URL_CURL_DOWNLOAD_SSL`: URL for speed test download.
- `URL_CURL_UPLOAD_SSL`: URL for speed test upload.
