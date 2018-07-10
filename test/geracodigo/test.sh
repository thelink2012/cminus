#!/bin/sh
GERACODIGO=../../geracodigo
tempfile=$(mktemp)
tempout=$(mktemp)
exit_code=0
for infile in *.in; do
    [ -f "$infile" ] || break
    stdin_file="${infile%.*}.stdin"
    stdout_file="${infile%.*}.stdout"

    printf "Testing $infile... "
    if $GERACODIGO "$infile" "$tempout" && spim -f "$tempout" < "$stdin_file" | tail -n +2 | diff - "$stdout_file" >$tempfile; then
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
