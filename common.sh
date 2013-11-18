#!/usr/bin/env bash
set -e
set -u

p="$1"
out="$2"

while read -r; do
    :
done < "$p"

pp_include="#include \"$p\""
