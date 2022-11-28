#!/bin/sh
./compile-wasm.sh
rsync -azP web badlogic@marioslab.io:/home/badlogic/marioslab.io/data/web/dump/lilray/