# HTTP 1.1 Server

## Features
- Compatible with RFC 9112 and RFC 9110
- Can render HTML, CSS, JS, PNG, JPEG, GIF, SVG and WEBP formats
- Can handle GET and POST requests
- gzip compression

## Downloading and Building
```
git clone --depth=1 https://github.com/phsyyshp/httpServer
cd httpServer
mkdir build
cd build
cmake ..
make
```
## Usage 
After building run the server with the following command:
```
./server --directory <full-path-to-website-files>
```
