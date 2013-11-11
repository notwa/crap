#!/bin/zsh
local bench="$1"
local against="$2"
local i x n=0 t m=9999
finish() {
    [ $n -gt 0 ] && printf "\nmin %.3f  avg %.3f  total %.3f\n" $m $((x/n)) $x
    exit 0
}
trap finish INT
echo -n "…"
for ((i=0; i<8; i++)); do
    sleep 0.5
    t="$(TIMEFMT='%*E'$'\n'; (time "$bench" "$against") 2>&1)"
    echo -n " ${t}"
    let 'x += t'
    let 'n += 1'
    [[ $t < $m ]] && m=$t
done
finish
