#!/usr/bin/env bash

build/lz77 -e $1 /tmp/test_encoded
build/lz77 -d /tmp/test_encoded /tmp/test_decoded;
diff -q $1 /tmp/test_decoded;
