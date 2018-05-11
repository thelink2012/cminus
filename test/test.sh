#!/bin/sh
set -e
echo "Testing scanner..."
cd lexico
sh ./test.sh
cd ..
echo "Testing parser..."
cd sintatico
sh ./test.sh
cd ..
