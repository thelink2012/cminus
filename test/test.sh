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
echo "Testing code generation..."
cd geracodigo
sh ./test.sh
cd ..
cd ..
