# fastrand

Fast random number generation in an interval in Python: Up to 10x faster than random.randint.

Blog post: [Ranged random-number generation is slow in Python](https://lemire.me/blog/2016/03/21/ranged-random-number-generation-is-slow-in-python/)




Usage... 

```python
import fastrand

print("generate an integer in [0,1001)")
fastrand.pcg32bounded(1001) 
print("generate an integer in [100,1000]")
fastrand.pcg32randint(100,1000) # requires Python 3.7 or better
print("Generate a random 32-bit integer.")
fastrand.pcg32()
print("Generate a number in [0,1).")
fastrand.pcg32_uniform()

if fastrand.SIXTYFOUR: # support for xorshift128+ is limited to some 64-bit platforms (linux, macos, etc.)
    print("generate an integer in [0,1001)")
    fastrand.xorshift128plusbounded(1001) 
    print("generate an integer in [100,1000]")
    fastrand.xorshift128plusrandint(100,1000) # requires Python 3.7 or better
    print("Generate a random 64-bit integer.")
    fastrand.xorshift128plus()
    print("Generate a number in [0,1).")
    fastrand.xorshift128plus_uniform()
```

We also include functions for fast float generation.

It is nearly an order of magnitude faster than the alternatives:

```

python3 -m timeit -s 'import fastrand' 'fastrand.pcg32bounded(1001)'
10000000 loops, best of 5: 23.6 nsec per loop

python3 -m timeit -s 'import fastrand' 'fastrand.pcg32randint(100,1000)'
10000000 loops, best of 5: 24.6 nsec per loop

python3 -m timeit -s 'import random' 'random.randint(0,1000)'
1000000 loops, best of 5: 216 nsec per loop

# if you have numpy:
python3 -m timeit -s 'import numpy' 'numpy.random.randint(0, 1000)'
500000 loops, best of 5: 955 nsec per loop

```

The pcg32 generator is a 32-bit generator so it generates values in the interval from `0` to `2**32-1`.
The xorshift128+ generator is a 64-bit generator so that it can generate values in a 64-bit range (up to `2**64-1`).


If you have Linux, macOS or Windows, you should be able to do just pip install...

```
pip install fastrand
```

You may need root access (sudo on macOS and Linux).

It is sometimes useful to install a specific version, you can do so as follows;

```
pip install fastrand==1.2.4
```



Generally, you can build the library as follows (if you have root):


```bash
python setup.py build
python setup.py install 
```

or

```bash
python setup.py build
python setup.py install --home=$HOME
export PYTHONPATH=$PYTHONPATH:~/lib/python
```


## Changing the seed and multiple streams

- You can change the seed with a function like `pcg32_seed`. The seed determines the random values you get. Be mindful that naive seeds (e.g., `int(time.time())`) can deliver poor initial randomness. A few calls to `pcg32()` may help to boost the improve the randomness. Or else you may try a better seed.
- If you need to produce multiple streams of random numbers, merely changing the seed is not enough. You are better off using different increments by calling the `pcg32inc`. The increments should all be distinct. Note that the least significant bit of the increment is always set to 1 no matter which value you pass: so make sure your increments are distinct 31-bit values (ignoring the least significant bit).
- You may also initialize xorshift128+ with `xorshift128plus_seed1` and `xorshift128plus_seed2`.

## Reference

* http://www.pcg-random.org
* Daniel Lemire, [Fast Random Integer Generation in an Interval](https://arxiv.org/abs/1805.10941), ACM Transactions on Modeling and Computer Simulation, Volume 29 Issue 1, February 2019 
