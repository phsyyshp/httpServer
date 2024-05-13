

#!/bin/sh
#
# DON'T EDIT THIS!
#
# CodeCrafters uses this file to test your code. Don't make any changes here!
#
# DON'T EDIT THIS!
set -e
cmake . -DCMAKE_BUILD_TYPE=DEBUG >/dev/null
make >/dev/null
sudo lsof -ti:4221 -sTCP:LISTEN | xargs sudo kill -9 || true
gdb --args ./server "$@"
