#!/bin/sh
SINTATICO=../../sintatico
tempfile=$(mktemp)
tempout=$(mktemp)
exit_code=0
for infile in *.in; do
    [ -f "$infile" ] || break
    outfile="${infile%.*}.out"

    printf "Testing $infile... "
    cat "$outfile" | tr -d '[:space:]' >$tempout
    if $SINTATICO "$infile" - | tr -d '[:space:]' | diff - "$tempout" >$tempfile; then
        printf "\033[0;32mOK\033[0m\n"
    else
        printf "\033[0;31mFAILED\033[0m\n"
        cat "$tempfile"
        exit_code=1
    fi
done
rm "$tempout"
rm "$tempfile"
exit $exit_code
