#!/bin/bash

cd ../../src
echo `pwd`
make clean
make
cd ../example/DataCheck
echo `pwd`
make clean
make
