#include <Python.h>

/*
pgc
*/

struct pcg_state_setseq_64 {    // Internals are *Private*.
    uint64_t state;             // RNG state.  All values are possible.
    uint64_t inc;               // Controls which RNG sequence (stream) is
    // selected. Must *always* be odd.
};
typedef struct pcg_state_setseq_64 pcg32_random_t;
static pcg32_random_t pcg32_global = { 0x853c49e6748fea9bULL, 0xda3e39cb94b95bdbULL };

static inline uint32_t pcg32_random_r(pcg32_random_t* rng) {
    uint64_t oldstate = rng->state;
    rng->state = oldstate * 6364136223846793005ULL + rng->inc;
    uint32_t xorshifted = (uint32_t)(((oldstate >> 18u) ^ oldstate) >> 27u);
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

static inline uint32_t pcg32_random(void) {
    return pcg32_random_r(&pcg32_global);
}


static inline void pcg32_init_state(uint64_t state) {
    pcg32_global.state = state;
}

static inline void pcg32_init_inc(uint64_t inc) {
    pcg32_global.inc = inc | 1;
}


static PyObject*
pcg32(PyObject* self, PyObject* args)
{
    return Py_BuildValue("I", pcg32_random());
}

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
#define PyInt_AsUnsignedLongLongMask(x) PyLong_AsUnsignedLongLongMask(x)
#endif

static PyObject*
pcg32bounded(PyObject* self, PyObject* args) {
    long n = PyInt_AsLong(args);
    if ((n > 0) && (n <= UINT32_MAX))
      return Py_BuildValue("I", pcg32_random_bounded_divisionless((uint32_t)n));
    if (!PyErr_Occurred()) {
      PyErr_SetString(PyExc_ValueError, "no such random number exist");
    }
    Py_RETURN_NONE;
}

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 7
static PyObject*
pcg32randint(PyObject* self, PyObject *const *args, Py_ssize_t len_args) {
    long n1 = PyInt_AsLong(args[0]);
    long n2 = PyInt_AsLong(args[1]);
    if ((n1 > UINT32_MAX) && (n2 > UINT32_MAX)) {
        PyErr_SetString(PyExc_ValueError, "the bounds of the interval cannot be greater than 2**32-1");
        Py_RETURN_NONE;
    }
    if(n2 < n1) {
        PyErr_SetString(PyExc_ValueError, "the second argument must be greater or equal than the first");
        Py_RETURN_NONE;
    }
    return Py_BuildValue("I", n1 + pcg32_random_bounded_divisionless((uint32_t)(n2 - n1 + 1)));
}
#endif

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



/**
* Vigna's
*/

uint64_t xorshift128plus_s[2]= {1,1};

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




static inline void xorshift128plus_init_state1(uint64_t state1) {
    xorshift128plus_s[0] = state1;
}



static inline void xorshift128plus_init_state2(uint64_t state2) {
    xorshift128plus_s[1] = state2;
}

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





static PyMethodDef FastRandMethods[] =
{
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 7
    {"pcg32randint", (PyCFunction)pcg32randint, METH_FASTCALL},
#endif
     {"xorshift128plus", xorshift, METH_NOARGS, "generate random integer (64 bits)"},
     {"pcg32", pcg32, METH_NOARGS, "generate random integer (32 bits) using PCG"},
     {"pcg32bounded", pcg32bounded, METH_O, "generate random integer in the interval [0,range) using PCG."},
     {"pcg32inc", pcg32inc, METH_O, "change the increment parameter of the pcg32 generator (global, for experts)."},
     {"pcg32_seed", pcg32state, METH_O, "seed the pcg32 generator (global)."},
     {"xorshift128plus_seed1", xorshift128plus_seed1, METH_O, "seed the xorshift128plus generator (global, first 64 bits)."},
     {"xorshift128plus_seed2", xorshift128plus_seed2, METH_O, "seed the xorshift128plus generator (global, second 64 bits)."},
     {NULL, NULL, 0, NULL}
};

#if PY_MAJOR_VERSION >= 3

static struct PyModuleDef cModFastrand =
{
    PyModuleDef_HEAD_INIT,
    "fastrand",
    "",
    -1,
    FastRandMethods
};


PyMODINIT_FUNC
PyInit_fastrand(void)
{
     return PyModule_Create(&cModFastrand);
}

#else

PyMODINIT_FUNC
initfastrand(void)
{
     (void) Py_InitModule("fastrand", FastRandMethods);
}

#endif
