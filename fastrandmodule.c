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
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

static inline uint32_t pcg32_random(void) {
    return pcg32_random_r(&pcg32_global);
}


static PyObject*
pcg32(PyObject* self, PyObject* args)
{
    return Py_BuildValue("i", pcg32_random());
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


static PyObject*
pcg32bounded(PyObject* self, PyObject* args)
{
    // int n;
    //if (!PyArg_ParseTuple(args, "i", &n))
    //    return NULL;
    long n = PyLong_AsLong(args);
    if (n <= 0) return PyErr_Occurred() ? NULL : (Py_INCREF(Py_None), Py_None);
    return Py_BuildValue("i", pcg32_random_bounded_divisionless(n));
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
    return Py_BuildValue("l", xorshift128plus());
}











static PyMethodDef FastRandMethods[] =
{
     {"xorshift128plus", xorshift, METH_NOARGS, "generate random integer (64 bits)"},
     {"pcg32", pcg32, METH_NOARGS, "generate random integer (32 bits) using PCG"},
     //{"pcg32bounded", pcg32bounded, METH_VARARGS, "generate random integer in the interval [0,range) using PCG."},
     {"pcg32bounded", pcg32bounded, METH_O, "generate random integer in the interval [0,range) using PCG."},
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
