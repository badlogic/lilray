#!/bin/sh
dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
pushd $dir > /dev/null
mkdir -p web/assets
cp assets/* web/assets
emcc \
	-Isrc \
	-O3 -fno-rtti -fno-exceptions -lc++abi -lc++ \
	-s STRICT=1 \
	-s ENVIRONMENT=web \
	-s LLD_REPORT_UNDEFINED \
	-s MODULARIZE=1 \
	-s ALLOW_MEMORY_GROWTH=1 \
	-s ALLOW_TABLE_GROWTH \
	-s MALLOC=emmalloc \
	-s EXPORT_ALL=1 \
	-s EXPORTED_FUNCTIONS='["_malloc", "_free"]' \
	-s EXPORTED_RUNTIME_METHODS=ccall,cwrap \
	--no-entry \
	-s EXPORT_NAME=liblilray \
	"src/lilray.cpp" "src/lilray-c.cpp" \
	-o web/liblilray.js
	ls -lah web
	python3 -m http.server --bind 127.0.0.1 --directory web
popd