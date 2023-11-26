#!/bin/bash
set -e

flags=$1

rm -v *.o compiler || true
g++ -std=c++2b *.cpp -o compiler $flags
