# fastrand
Fast random number generation in Python

Blog post: [Ranged random-number generation is slow in Python…](https://lemire.me/blog/2016/03/21/ranged-random-number-generation-is-slow-in-python/)

Requirements: Linux-like system (including a Mac), a C compiler, Python.

```
python setup.py build
python setup.py install --home=$HOME
PYTHONPATH=$PYTHONPATH:~/lib/python
```

It is nearly an order of magnitude faster than the alternatives:

```
python -m timeit -s 'import fastrand' 'fastrand.pcg32bounded(1001)'
10000000 loops, best of 3: 0.0914 usec per loop
python -m timeit -s 'import random' 'random.randint(0,1000)'
1000000 loops, best of 3: 0.698 usec per loop
python -m timeit -s 'import numpy' 'numpy.random.randint(0, 1000)'
1000000 loops, best of 3: 0.795 usec per loop
```

## future work

Also look at https://github.com/rkern/line_profiler

## Reference

* Daniel Lemire, [Fast Random Integer Generation in an Interval](https://arxiv.org/abs/1805.10941), ACM Transactions on Modeling and Computer Simulation (to appear)
