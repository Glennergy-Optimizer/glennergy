#!/bin/sh
set -e


if [ -f Makefile ]; then
    echo "Uninstalling"
    sudo make uninstall
fi

# Added excludes for doxygen folders
find . -type f -name Makefile \
  -not -path "*/latex/*" \
  -not -path "*/html/*" \
  -not -path "*/Docs/*" \
  -not -path "*/Client-CPP/*" \
  | while read mf; do
    dir=$(dirname "$mf")
    echo "Processing $dir"
    sudo make -C "$dir" uninstall

done