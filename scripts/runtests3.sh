#!/bin/sh
set -o verbose
python3 setup.py build 
python3 setup.py install --home=$HOME
export PYTHONPATH=$PYTHONPATH:$HOME/lib/python
python3 -m timeit -s 'import fastrand' 'fastrand.pcg32bounded(1001)'
python3 -m timeit -s 'import fastrand' 'fastrand.pcg32randint(100,1000)'
python3 -m timeit -s 'import random' 'random.randint(0,1000)'
python3 -m timeit -s 'import numpy' 'numpy.random.randint(0, 1000)'
