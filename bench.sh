#!/bin/bash
bench="$1"
against="$2"

TIMEFORMAT='%3R'
for i in {1..8}; do
    time "$bench" "$against"
done 2>&1 >/dev/null | awk 'BEGIN{m=999;printf " …\033[90m"}
{a+=$1;n++;m=$1<m?$1:m;printf " %6.3f",$1}
END{printf "\033[0m\nmin %6.3f  —  avg %6.3f  —  total %7.3f\n",m,a/n,a}'
