# fastrand
Fast random number generation in Python

Not ready for actual use. Experimental.

Requirements: Linux-like system, a C compiler, Python.

```
python setup.py build
python setup.py install --home=~
PYTHONPATH=$PYTHONPATH:~/lib/python
```


```
$ python -m timeit -s 'import fastrand' 'fastrand.pcg32bounded(1001)'
10000000 loops, best of 3: 0.0838 usec per loop

$ python -m timeit -s 'import random' 'random.random()'
10000000 loops, best of 3: 0.0453 usec per loop

$ python -m timeit -s 'import random' 'random.randint(0,1000)'
1000000 loops, best of 3: 0.672 usec per loop

$ python -m timeit -s 'import random' 'int(random.random() * 1001)'
10000000 loops, best of 3: 0.145 usec per loop
```


