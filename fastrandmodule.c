#include <Python.h>
#include <stdint.h>
#include <limits.h>
#include <stddef.h>

/***
 * PGC32
 * https://www.pcg-random.org/
 */

struct pcg_state_setseq_64 {    // Internals are *Private*.
    uint64_t state;             // RNG state.  All values are possible.
    uint64_t inc;               // Controls which RNG sequence (stream) is
    // selected. Must *always* be odd.
};
// This is the global state for the PCG32 generator.
typedef struct pcg_state_setseq_64 pcg32_random_t;
static pcg32_random_t pcg32_global = { 0x853c49e6748fea9bULL, 0xda3e39cb94b95bdbULL };

// Generate a random number in the range [0, 2**32-1]
// This is the PCG32 algorithm, which is a fast and efficient random number generator.
static inline uint32_t pcg32_random_r(pcg32_random_t* rng) {
    uint64_t oldstate = rng->state;
    rng->state = oldstate * 6364136223846793005ULL + rng->inc;
    uint32_t xorshifted = (uint32_t)(((oldstate >> 18u) ^ oldstate) >> 27u);
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

// Generate a random number in the range [0, 2**32-1]
// This is a wrapper function that uses the global state.
static inline uint32_t pcg32_random(void) {
    return pcg32_random_r(&pcg32_global);
}

// Initialize the PCG32 generator with a state and increment.
static inline void pcg32_init_state(uint64_t state) {
    pcg32_global.state = state;
}

// Initialize the PCG32 generator with an increment.
static inline void pcg32_init_inc(uint64_t inc) {
    pcg32_global.inc = inc | 1;
}

static PyObject*
pcg32(PyObject* self, PyObject* args)
{
    return Py_BuildValue("I", pcg32_random());
}

// Generate a random number in the range [0, range)
static inline uint32_t pcg32_random_bounded_divisionless(uint32_t range) {
    uint64_t random32bit, multiresult;
    uint32_t leftover;
    uint32_t threshold;
    random32bit =  pcg32_random();
    multiresult = random32bit * range;
    leftover = (uint32_t) multiresult;
    if(leftover < range ) {
        threshold = -range % range ;
        while (leftover < threshold) {
            random32bit =  pcg32_random();
            multiresult = random32bit * range;
            leftover = (uint32_t) multiresult;
        }
    }
    return multiresult >> 32; // [0, range)
}


#if PY_MAJOR_VERSION >= 3
#define PyInt_AsLong(x)  PyLong_AsLong(x)
#define PyInt_AsUnsignedLongLong(x) PyLong_AsUnsignedLongLong(x)
#define PyInt_AsUnsignedLongLongMask(x) PyLong_AsUnsignedLongLongMask(x)
#endif // PY_MAJOR_VERSION >= 3

static PyObject*
pcg32bounded(PyObject* self, PyObject* args) {
    long n = PyInt_AsLong(args);
    if (PyErr_Occurred()) {
      PyErr_SetString(PyExc_ValueError, "I failed the represent the parameter as a long");
      Py_RETURN_NONE;
    }
    if ((n > 0) && (n <= UINT32_MAX))
      return Py_BuildValue("I", pcg32_random_bounded_divisionless((uint32_t)n));
    if (!PyErr_Occurred()) {
      PyErr_SetString(PyExc_ValueError, "the parameter should be between 1 and 2**32-1");
    }
    Py_RETURN_NONE;
}

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 7
static PyObject*
pcg32randint(PyObject* self, PyObject *const *args, Py_ssize_t len_args) {
    long n1 = PyInt_AsLong(args[0]);
    if (PyErr_Occurred()) {
      PyErr_SetString(PyExc_ValueError, "I failed the represent the first parameter as a long");
      Py_RETURN_NONE;
    }
    long n2 = PyInt_AsLong(args[1]);
    if (PyErr_Occurred()) {
      PyErr_SetString(PyExc_ValueError, "I failed the represent the second parameter as a long");
      Py_RETURN_NONE;
    }
    if(n2 < n1) {
        PyErr_SetString(PyExc_ValueError, "the second argument must be greater or equal than the first");
        Py_RETURN_NONE;
    }
    if (((n2 - n1) > UINT32_MAX - 1)) {
        PyErr_SetString(PyExc_ValueError, "the interval cannot be greater than 2**32-1");
        Py_RETURN_NONE;
    }
    return Py_BuildValue("I", n1 + pcg32_random_bounded_divisionless((uint32_t)(n2 - n1 + 1)));
}
#endif // #if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 7

static PyObject*
pcg32inc(PyObject* self, PyObject* args) {
    uint64_t n = PyInt_AsUnsignedLongLongMask(args);
    pcg32_init_inc(n);
    Py_RETURN_NONE;
}

static PyObject*
pcg32state(PyObject* self, PyObject* args) {
    uint64_t n = PyInt_AsUnsignedLongLongMask(args);
    pcg32_init_state(n);
    Py_RETURN_NONE;
}

static PyObject*
pcg32_uniform(PyObject* self, PyObject* args) {
    uint32_t r = pcg32_random();
    double result = (double)r * 0x1p-32; // Divide by 2^32
    return Py_BuildValue("d", result);
}

/**
* Vigna's xorshift128+ generator
* http://xorshift.di.unimi.it/xorshift128plus.c
*/

// This is the global state for the xorshift128+ generator.
uint64_t xorshift128plus_s[2]= {3678629397846363829,2900481245226533291};

// Generate a random number in the range [0, 2**64-1]
//http://xorshift.di.unimi.it/xorshift128plus.c
uint64_t xorshift128plus(void) {
    uint64_t s1 = xorshift128plus_s[0];
    const uint64_t s0 = xorshift128plus_s[1];
    xorshift128plus_s[0] = s0;
    s1 ^= s1 << 23; // a
    xorshift128plus_s[1] = s1 ^ s0 ^ (s1 >> 18) ^ (s0 >> 5); // b, c
    return xorshift128plus_s[1] + s0;
}

static PyObject*
xorshift(PyObject* self, PyObject* args)
{
    return Py_BuildValue("K", xorshift128plus());
}

// Seed the xorshift128+ generator with a 64-bit state.
static inline void xorshift128plus_init_state1(uint64_t state1) {
    xorshift128plus_s[0] = state1;
}

// Seed the xorshift128+ generator with a 64-bit state.
static inline void xorshift128plus_init_state2(uint64_t state2) {
    xorshift128plus_s[1] = state2;
}

#if defined(__SIZEOF_INT128__) && INTPTR_MAX == INT64_MAX

// Generate a random number in the range [0, range)
static inline uint64_t xorshift128plus_random_bounded_divisionless(uint64_t range) {
    uint64_t random64bit;
    __uint128_t multiresult;
    uint64_t leftover;
    uint64_t threshold;
    random64bit =  xorshift128plus();
    multiresult = (__uint128_t)random64bit * range;
    leftover = (uint64_t) multiresult;
    if(leftover < range ) {
        threshold = -range % range ;
        while (leftover < threshold) {
            random64bit =  xorshift128plus();
            multiresult = (__uint128_t)random64bit * range;
            leftover = (uint64_t) multiresult;
        }
    }
    return multiresult >> 64; // [0, range)
}

static PyObject*
xorshift128plusbounded(PyObject* self, PyObject* args) {
    uint64_t n = (uint64_t)PyInt_AsUnsignedLongLong(args);
    if (PyErr_Occurred()) {
      PyErr_SetString(PyExc_ValueError, "I failed the represent the parameter as a ulonglong");
      Py_RETURN_NONE;
    }
    return Py_BuildValue("K", xorshift128plus_random_bounded_divisionless(n));
}
#endif // #if defined(__SIZEOF_INT128__) && INTPTR_MAX == INT64_MAX

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 7 && INTPTR_MAX == INT64_MAX
static PyObject*
xorshift128plusrandint(PyObject* self, PyObject *const *args, Py_ssize_t len_args) {
    uint64_t n1 = (uint64_t)PyInt_AsUnsignedLongLong(args[0]);
    if (PyErr_Occurred()) {
      PyErr_SetString(PyExc_ValueError, "I failed the represent the first parameter as a ulonglong");
      Py_RETURN_NONE;
    }
    uint64_t n2 = (uint64_t)PyInt_AsUnsignedLongLong(args[1]);
    if (PyErr_Occurred()) {
      PyErr_SetString(PyExc_ValueError, "I failed the represent the second parameter as a ulonglong");
      Py_RETURN_NONE;
    }
    if(n2 < n1) {
        PyErr_SetString(PyExc_ValueError, "the second argument must be greater or equal than the first");
        Py_RETURN_NONE;
    }
    if (n2 - n1 == UINT64_MAX) {
        PyErr_SetString(PyExc_ValueError, "the interval cannot be greater than 2**64-1");
        Py_RETURN_NONE;
    }
    return Py_BuildValue("K", n1 + xorshift128plus_random_bounded_divisionless((uint64_t)(n2 - n1 + 1)));
}
#endif // PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 7 && INTPTR_MAX == INT64_MAX

static PyObject*
xorshift128plus_seed1(PyObject* self, PyObject* args) {
    uint64_t n = PyInt_AsUnsignedLongLongMask(args);
    xorshift128plus_init_state1(n);
    Py_RETURN_NONE;
}

static PyObject*
xorshift128plus_seed2(PyObject* self, PyObject* args) {
    uint64_t n = PyInt_AsUnsignedLongLongMask(args);
    xorshift128plus_init_state2(n);
    Py_RETURN_NONE;
}

static PyObject*
xorshift128plus_uniform(PyObject* self, PyObject* args) {
    uint64_t r = xorshift128plus();
    double result = (double)(r & 0x1FFFFFFFFFFFFF) * 0x1p-53; // Divide by 2^53
    return Py_BuildValue("d", result);
}

// This is the module method table.
// It defines the functions that will be available in the module.
static PyMethodDef FastRandMethods[] =
{
#if defined(__SIZEOF_INT128__) && INTPTR_MAX == INT64_MAX
    {"xorshift128plusbounded", xorshift128plusbounded, METH_O, "Generate random integer in the interval [0,range) using xorshift128+. The interval should be no wider than 2**64-1 and the parameter must be representable as a 64-bit integer. This function is only available on 64-bit platforms."},
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 7
    {"xorshift128plusrandint", (PyCFunction)xorshift128plusrandint, METH_FASTCALL, "Generate random integer in the interval provided using xorshift128+. The interval should be no wider than 2**64-1 and the parameters must be representable as 64-bit integers. This function is only available on 64-bit platforms."},
#endif // PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 7
#endif // defined(__SIZEOF_INT128__) && INTPTR_MAX == INT64_MAX
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 7
    {"pcg32randint", (PyCFunction)pcg32randint, METH_FASTCALL, "Generate random integer in the interval provided using PCG32. The interval should be no wider than 2**32-1 and the parameters must be representable as a long which is a 64-bit integer on 64-bit platforms."},
#endif // PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 7
     {"xorshift128plus", xorshift, METH_NOARGS, "generate random integer (64 bits) using xorshift128+"},
     {"xorshift128plus_uniform", xorshift128plus_uniform, METH_NOARGS, "Generate a random double in [0, 1) using xorshift128+."},
     {"pcg32", pcg32, METH_NOARGS, "Generate random integer (32 bits) using PCG32, a 32-bit integer is no larger than 2**32-1."},
     {"pcg32bounded", pcg32bounded, METH_O, "Generate random integer in the 32-bit interval [0,range) using PCG32. A 32-bit integer is no larger than 2**32-1."},
     {"pcg32_uniform", pcg32_uniform, METH_NOARGS, "Generate a random double in [0, 1) using PCG32."},
     {"pcg32inc", pcg32inc, METH_O, "change the increment parameter of the pcg32 generator (global, for experts)."},
     {"pcg32_seed", pcg32state, METH_O, "seed the pcg32 generator (global)."},
     {"xorshift128plus_seed1", xorshift128plus_seed1, METH_O, "seed the xorshift128+ generator (global, first 64 bits)."},
     {"xorshift128plus_seed2", xorshift128plus_seed2, METH_O, "seed the xorshift128+ generator (global, second 64 bits)."},
     {NULL, NULL, 0, NULL}
};

#if PY_MAJOR_VERSION >= 3
// This is the module definition structure.
// It defines the name, docstring, and methods of the module.
static struct PyModuleDef cModFastrand =
{
    PyModuleDef_HEAD_INIT,
    "fastrand",
    "Fast random number generation in an interval in Python: Up to 10x faster than the standard library", // module documentation,
    -1,
    FastRandMethods
};


PyMODINIT_FUNC
PyInit_fastrand(void)
{
    PyObject *module = PyModule_Create(&cModFastrand);
    PyModule_AddStringConstant(module, "__doc__", "Fast random number generators in an interval."); 
#if defined(__SIZEOF_INT128__) && INTPTR_MAX == INT64_MAX
    PyModule_AddIntConstant(module, "SIXTYFOUR", 1);
#else // defined(__SIZEOF_INT128__) && INTPTR_MAX == INT64_MAX
    PyModule_AddIntConstant(module, "SIXTYFOUR", 0);
#endif // defined(__SIZEOF_INT128__) && INTPTR_MAX == INT64_MAX
    return module;
}

#else // PY_MAJOR_VERSION >= 3

PyMODINIT_FUNC
initfastrand(void)
{
     (void) Py_InitModule("fastrand", FastRandMethods);

}

#endif // PY_MAJOR_VERSION >= 3
