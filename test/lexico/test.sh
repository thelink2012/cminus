#!/bin/sh
LEXICO=../../lexico
tempfile=$(mktemp)
exit_code=0
for infile in *.in; do
    [ -f "$infile" ] || break
    outfile="${infile%.*}.out"

    printf "Testing $infile... "
    if $LEXICO "$infile" - | diff - "$outfile" >$tempfile; then
        printf "\033[0;32mOK\033[0m\n"
    else
        printf "\033[0;31mFAILED\033[0m\n"
        cat "$tempfile"
        printf "\n"
        exit_code=1
    fi
done
rm "$tempfile"
exit $exit_code
