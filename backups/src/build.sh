#!/bin/bash
set -eu

rm -v *.o compiler || true
g++ -std=c++2b *.cpp -o compiler
