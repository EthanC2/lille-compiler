#!/bin/bash

rm -v *.o compiler
g++ -std=c++2b *.cpp -o compiler
