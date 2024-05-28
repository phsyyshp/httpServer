#!/bin/sh
set -e
cmake . -DCMAKE_BUILD_TYPE=Release >/dev/null
make >/dev/null
sudo lsof -ti:4221 -sTCP:LISTEN | xargs sudo kill -9 || true
exec ./server "$@"
