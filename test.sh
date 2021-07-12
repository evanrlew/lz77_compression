#!/usr/bin/env bash

test_files=$1
if [[ $1 == "" ]]; then
  test_files=tests/assets/*
fi

for f in $test_files; do 
  build/lz77 -e $f /tmp/test_encoded
  build/lz77 -d /tmp/test_encoded /tmp/test_decoded;
  diff -q $f /tmp/test_decoded || exit 1;
  enc_wc=$(wc -c /tmp/test_encoded | grep -Eo "(\d+)")
  dec_wc=$(wc -c /tmp/test_decoded | grep -Eo "(\d+)")
  comp_ratio=$(echo "scale=3;$enc_wc/$dec_wc" | bc)
  echo "(passed) $f: $enc_wc/$dec_wc = $comp_ratio"
done
