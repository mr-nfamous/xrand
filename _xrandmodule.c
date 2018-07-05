
/*
Copyright 2018 Dan Snider

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, 
this list of conditions and the following disclaimer in the documentation 
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS 
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR 
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN 
IF ADVISEDOF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
* Added `load_rng` to fill the ext module's dict. 
* I hate dicts. It takes me hours to remember how to deal with references
* with them, so there's an extra rng and 1/2 the methods there should be.
* RAND_METHODS SHOULD keep a true backup so they can be placed back into
* the module if they're deleted, yet Py_REFCNT(rng) == 38 after it's done.

* The entire type initialization process needs refactoring, but before that
* can happen, xrand_auto needs to be refactored. And before THAT can happen,
* the rv_iter argument structs need to be refactored...which probably leads
* to another thing and so on and so forth.

* In the interim, this stuff could be fixed now:
* - Remove pointless error checking remnants
* - The py_int_from_x series needs to switch from _PyLong_New to __int__
* - Add the zero argument macros for dice and choices
* - Do something about the function decorators that have piled up..
* - The "useless" looking stuff isn't; they're just needed for tests,
*   otherwise I'd say that could be done. And those tests are mega low priority.
*
*/    
#include "Python.h"

#include <stdbool.h>
#include <stdint.h>
#include <math.h> 

#ifndef XRAND_TYPES_H
#include "types.h"
#include "auto.h"
#endif
#include "data.h"
DATA(double const) RAND_FLOAT_MIN = 1.0 / 4503599627370496.0;
DATA(double const) RAND_FLOAT_MAX = 0.9999999999999998;
DATA(double const, PI) = 3.141592653589793;
DATA(double const, TAU) = 6.283185307179586;
DATA(char, SEED_BITS_STR)[3];
DATA(rand64_t) INIT_SEED;
DATA(rand64_t) SEED_CTR;
DATA(const int8_t, BIT_LENGTHS[]) = { 
    0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
};

DATA(Py) small_ints[TINY_INTS];
DATA(Py) small_strs[CACHED_CHARS];
DATA(Py) *SMALL_INTS;
DATA(Py) *LATIN1;
DATA(Py) UNICODE_EMPTY;
DATA(Py) EMPTY_TUPLE;
DATA(hashfunc) UNICODE_HASH;
DATA(richcmpfunc) dict_richcomp;

#ifndef RAND_ERR_MESSAGES
    c_str no_homo    = "'%s' object is not a valid type for weights";
    c_str no_homo_i  = "non-int weighting of type '%s'";
    c_str no_homo_d  = "non-float weighting of type '%s'";
    c_str zero_w     = "weights must be positive, non-zero numbers";
    c_str empty_pop  = "cannot choose from an empty population";
    c_str bad_wflote = "float-based weights must be finite";
    c_str bad_wint   = "sum of integer-based weights overflow C size_t";
#endif
#include "abstract.h"
FUNC(py_type*) get_tp_layout(Py ob) {
    py_type* base = Py_TYPE(ob);
    while(base && PyType_HeapType(base)) {
        py_type *next = base->tp_base;
        base = next;
    }
    return base;
}
FUNC(popentry*) new_popentry(Py key) {
    py_type *base;
    popentry *entry;
    if(PopEntry_Check(key))
        return (popentry*)Py_NEWREF(key);
    base = Py_TYPE(key);
    entry = __object__(&PopEntry_Type);
    if(!entry)
        return NULL;
    PopEntry_BASE(entry) = base;
    if(base->tp_hash && base->tp_hash != PY_NOHASH) {
        PopEntry_HASH(entry) = base->tp_hash(key);
        if(PopEntry_HASH(entry) == -1) {
            Py_DECREF(entry);
            return NULL;
        }
    }
    else {
        base = get_tp_layout(key);
        PopEntry_HASH(entry) =_Py_HashPointer(base);
    }
    Py_INCREF(base);
    PopEntry_ITEM(entry) = Py_NEWREF(key);
    PopEntry_COMP(entry) = base->tp_richcompare;
    return entry;
}
FUNC(Py) popentry_new(py_type* tp, Py args, Py kws) {
    if(!no_kwargs("PopEntry", kws))
        return NULL;
    if(Py_SIZE(args) != 1)
        return PyTypeError("PopEntry() requires one argument");
    return (Py)new_popentry(TUPLE_ITEMS(args)[0]);
}
VOID_FUNC       (popentry_dealloc) (popentry *entry) {
    Py_DECREF(PopEntry_ITEM(entry));
    Py_DECREF(PopEntry_BASE(entry));
    PyObject_Del(entry);
}
HASH_FUNC       (popentry_hash)    (void *entry) {
    return PopEntry_HASH(entry);
}
IMPL_FUNC(Py)    popentry_richcmp  (Py self, Py other, int const op) {
    Py cmp, a, b;
    int eq = ((void*)self) == ((void*)other);

    if(eq)
        Py_RETURN_TRUTH(op==Py_EQ);
    a = PopEntry_ITEM(self);
    b = PopEntry_Check(other)? PopEntry_ITEM(other): ((Py)other);
    cmp = PopEntry_COMP(self)(a, b, 1);
    if(!cmp || ERROR())
        return NULL;
    eq = cmp==Py_True;
    Py_DECREF(cmp);

    switch(op) {
        case Py_NE: Py_RETURN_TRUTH(!eq);
        case Py_EQ: Py_RETURN_TRUTH(eq);
        case Py_LE: break;
        case Py_GE: break;
        case Py_LT: break;
        case Py_GT: break;
        default: return PySystemError("unsuported richcomp flag");
    }
    Py_RETURN_NOTIMPLEMENTED;
}
IMPL_FUNC(Py)    popentry_repr     (Py self) {
    return __format__("<PopEntry: %R>", PopEntry_ITEM(self));
}
IMPL_FUNC(Py) pop_richcmp(Py self, Py other, int const op) {

    Py a;

    if(!((op==Py_EQ)||(op==Py_NE)))
        Py_RETURN_NOTIMPLEMENTED;
    if(self==other)
        Py_RETURN_TRUTH(op==Py_EQ);
    if(Population_Check(other))
        other = Population_MP(other);
    if(PyDict_Check(other))
        a = dict_richcomp(Population_MP(self), other, op);
    else
        a = Py_TRUTH(op==Py_NE);
    return a;
}
VOID_FUNC     (pop_dealloc)     (Population *pop) {
    Py_XDECREF(pop->mp);
    PyObject_Del(pop);
}
SSIZEFUNC     (pop_length)      (Population *pop) {
    return PyDict_GET_SIZE(pop->mp);
}
IMPL_FUNC(Py)  pop_get_item     (Population *pop, Py key) {
    Py value;
    if(PopEntry_Check(key))
        Py_INCREF(key);
    else {
        popentry *entry = new_popentry(key);
        if(!entry)
            return NULL;
        key = (Py)entry;
    }
    value = _PyDict_GetItem_KnownHash(pop->mp, key, PopEntry_HASH(key));
    Py_DECREF(key);
    Py_XINCREF(value);
    return value;
}
IMPL_FUNC(int) pop_contains     (Population *pop, Py key) {
    Py value = pop_get_item(pop, key);
    int result =  value? 1: (ERROR()? -1: 0);
    Py_XDECREF(value);
    return result;
}
IMPL_FUNC(Py)  pop_cleanup      (PySequence *sq, Py need_dec, Py mp) {
    if(need_dec)
        Py_DECREF(need_dec);;
    if(sq)
        PySeq_CLEAR(sq);
    return mp;
}
IMPL_FUNC(int) pop_check_fast_pair   (PySequence *sq) {
    static char const msg[] = (
        "Population() requires a dict or sequence of (k,v) pairs");
    if(pyseq_SIZE(sq) != 2)
        return PyTypeError(msg), 0;
    return 1;
}
IMPL_FUNC(int) pop_parse_member (PySequence *sq, Py mp, size_t index) {

    popentry *key;
    Py entry, k, v;
    PySequence _pq = PySequence_new();
    PySequence *pq = &_pq;
    int result = -1;

    #ifdef RAND_DEBUG
    if(index > pyseq_SIZE(sq))
        return PySystemError("yeah index bigger %zu", index), -1;
    #endif
    
    entry =  FastSequence_OB_ITEM(sq)[index];

    if(!py_sequence_parse(entry, pq, 1))
        return -1;
    if(!pop_check_fast_pair(pq))
        goto X;

    k = PySequence_item(pq, 0);
    if(!k)
        goto X;
    v = PySequence_item(pq, 1);
    if(!v)
        goto X;
    key = new_popentry(k);
    if(!key)
        goto X;
    result =  _PyDict_SetItem_KnownHash(mp, (Py)key, v, PopEntry_HASH(key));
    Py_DECREF(key);
    X:
    Py_XDECREF(k);
    Py_XDECREF(v);
    PySeq_CLEAR(pq);
    return result;
}
IMPL_FUNC(Py)  pop_repr         (Population *self) {
    return __format__("Population(%R)", self->mp);
}
FUNC(Py)  population_get_mp(Py table) {

    Py           mp;
    PySequence  _sq;
    PySequence  *sq;

    if(Population_Check(table))
        return Py_NEWREF(((Population*)table)->mp);
    if(Py_TYPE(table)== &PyDictProxy_Type)
        table = ((py_dict_proxy*)table)->mp;
    if(PyDict_Check(table))
        return Py_NEWREF(table);

    _sq = PySequence_new();
    sq = &_sq;

    if( !py_sequence_parse(table, sq, 1 ))
        return NULL;

    if(!PySeq_AS_FAST(sq))
        return pop_cleanup(sq, NULL, NULL);

    if(!(mp = PyDict_New()))
        return pop_cleanup(sq, NULL, NULL);

    if(!pyseq_SIZE(sq))
        return pop_cleanup(sq, NULL, mp);

    for(size_t i=0; i<pyseq_SIZE(sq); ++i)
        if(pop_parse_member(sq, mp, i))
            return pop_cleanup(sq, mp, NULL);
    if(!pyseq_SQ(sq))
        return NULL;
    return pop_cleanup(sq, NULL, mp);
}
FUNC(Py) init_pop_from_kws(Py kw) {
    Py mp;
    if(!kw)
        return PyDict_New();
    mp = PyDict_Copy(kw);
    if(!mp)
        return NULL;
    return population_get_mp(mp);
}
FUNC(Py) init_pop_from_args(Py args) {
    return population_get_mp(TUPLE_ITEMS(args)[0]);
}
FUNC(Py) init_pop_from_both(Py args, Py kws) {
    Py t, it, mp;
    mp = init_pop_from_kws(kws);
    if(!mp)
        return NULL;
    it = init_pop_from_args(args);
    t = ((it==NULL) || (_PyDict_MergeEx(it, mp, 2) != 0))? NULL: it;
    Py_DECREF(mp);
    return t;
}
FUNC(Py) population_new(py_type *tp, Py args, Py kws) {

    Population *pop;

    if(!args)
        return PySystemError("null arguments");
    if(Py_SIZE(args) > 1)
        return PyTypeError("Population expects at most 1 argument");

    pop = __object__(&Population_Type);
    if(!pop)
        return NULL;

    switch(Py_SIZE(args)) {
        case 0:
            pop->mp = init_pop_from_kws(kws);
            break;
        case 1:

            if(kws)
                pop->mp = init_pop_from_both(args, kws);
            else
                pop->mp = init_pop_from_args(args);
    }
    if((pop->mp))
        return (Py)pop;
    Py_DECREF(pop);
    return NULL;
}
#include "generator.h"
#if defined(RANDOM_METHODS_DEFINED)||1
  
RAND_ARGT(betavariate) {
    double alpha, beta;
} args_t(betavariate);
RAND_ARGT(expovariate) {
    double lambda;
    ssize_t num;
} args_t(expovariate);
RAND_ARGT(gammavariate) {
    double alpha, beta;
} args_t(gammavariate);
RAND_ARGT(normalvariate) {
    double mu, sigma;
} args_t(normalvariate);
RAND_ARGT(vonmisesvariate) {
    double mu, kappa;
} args_t(vonmisesvariate);
RAND_ARGT(paretovariate) {
    double alpha;
} args_t(paretovariate);
RAND_ARGT(triangular) {
    double lo, hi, c;
    int idk;
} args_t(triangular);
RAND_ARGT(uniform) {
    double a, b;
} args_t(uniform);
RAND_ARGT(weibullvariate) {
    double alpha, beta;
} args_t(weibullvariate);
RAND_ARGT(shuffled) {
    PySequence *sq;
    Py mem;
} args_t(shuffled);
RAND_ARGT(rand_index) {
    sizes_t max;
    uint8_t open;
    uint8_t   sh;
    uint8_t   pb;

} args_t(rand_index);
typedef enum rv_flags {
    RV_UNUSED,
    RV_BETA,
    RV_EXPO,
    RV_GAMMA,
    RV_LOGNORM,
    RV_NORM,
    RV_PARETO,
    RV_TRI,
    RV_UNI,
    RV_VONMISES,
    RV_WEIBULL
} RV_FLAG;
union randiter_args_t {
#if 1
#define RANDITER_BUF_SIZE      sizeof(randiter_args_t)
#define _riter_flip_args_(o)   _riter_args_(o)->flip
#define randiter_FLIP(o)       _riter_flip_args_(o)
#define randiter_FLIP_HEADS(o) _riter_flip_args_(o).heads
#define randiter_FLIP_TAILS(o) _riter_flip_args_(o).tails
#define randiter_FLIP_ODDS(o)  _riter_flip_args_(o).odds
#define randiter_FLIP_FUNC(o)  _riter_func_(o)(randiter_RNG(o), &randiter_FLIP(o))

#define _riter_as_statf_(o)   _riter_args_(o)->statf
#define _riter_stat_args_(o)  _riter_args_(o)->statf.impls
#define randiter_STAT_FLAG(o) _riter_args_(o)->statf.impl
#define randiter_STAT(o)      _riter_stat_args_(o)
#define randiter_STAT_BETA(o) _riter_stat_args_(o).beta
#endif
    struct {
        Py heads;
        Py tails;
        double odds;
    } flip;
    struct {
        union {
            betavariate_args_t         beta;
            expovariate_args_t         expo;
            gammavariate_args_t       gamma;
            normalvariate_args_t    lognorm;
            normalvariate_args_t       norm;
            paretovariate_args_t     pareto;
           #define  triangularvariate_args_t triangular_args_t
            triangular_args_t    triangular;
            uniform_args_t          uniform;
            vonmisesvariate_args_t vonmises;
            weibullvariate_args_t   weibull;
        } impls;
        RV_FLAG impl;

    } statf;
};
struct randiter_funcs {
    riternext     next;
    reprfunc      repr;
    co_destr     destr;
    roll_impl alt_next;
};
struct randiter {
#if 1
    #define _riter_(o) ((randiter*)(o))
    #define _riter_buf_(o) ((randiter*)(o))->buf
    #define _riter_args_(o) ((randiter_args_t*)(((randiter*)(o))->buf))
    #define _riter_func_(o, f) _riter_(o)->f
    #define _riter_call_(o, ...) _riter_func_(o)(o, __VA_ARGS__)
    #define randiter_RNG(o)   _riter_(o)->rng
    #define randiter_ARGS(o)  _riter_args_(o)
    #define randiter_COUNTER(o) _riter_(o)->counter

    #define randiter_NEXT(o)       _riter_(o)->next
    #define randiter_REPR(o)       _riter_(o)->repr
    #define randiter_DESTR(o)      _riter_(o)->destr
    #define randiter_ALT_NEXT(o)   _riter_(o)->alt_next
    #define randiter_CALL_NEXT(o)  _riter_func_(o, next)(o)
    #define randiter_CALL_REPR(o)  _riter_func_(o, repr)((Py)o)
    #define randiter_CALL_DESTR(o) _riter_func_(o, destr)(o)
    #define randiter_CALL_ALT(o, tp) random_betavariate_impl(NULL,NULL)
#endif
    PyObject_HEAD
    RandomObject  *rng;
    ssize_t    counter;
    riternext     next;
    reprfunc      repr;
    co_destr     destr;
    roll_impl alt_next;
    char buf[RANDITER_BUF_SIZE];
};
struct randiter_setup_args{
    Py                       args;
    randiter_args_t      defaults;
    const randiter_funcs   *funcs;
    randiter_unpacker    unpacker;
};
FUNC(int)    rand_choice     (RNGSELF, choice_args_t *a, Py *r, size_t n) {
    sizes_t rv;
    for(size_t i = 0; i < n; ++i) {
        rv.u = next_index(rng, pyseq_SIZES(&a->sq));
        r[i] = PySequence_item(&a->sq, rv.i);
        if(!r[i])
            return 0;
    }
    return 1;
}
FUNC(int)    rand_zeroes     (RNGSELF, void *a, Py *r, size_t n) {
    for(size_t i=0; i<n; ++i)
        r[i] = Py_NEWREF(SMALL_INTS[0]);
    return 1;
}
FUNC(int)    rand_wsample    (RNGSELF, wsample_args *ax, Py *r, size_t k) {

    sizes_t rv;
    wchoice_args_t *a = ax->wc;

    #if defined(RAND_DEBUG)
    if(a->bisect_as != 'i')
        return PySystemError("sample doesn't work with floats"), 0;
    #endif

    for(size_t i=0, j=0; i<k; ++i) {
        do {
            rv.u = next_index(rng, &a->sum.c);
        } while(set_add_entry(ax->indices, &rv) != 1);
        j = bisect_size_t(a->mem, rv.u, a->hi);
        r[i] = Py_NEWREF(LIST_ITEMS(a->pop)[j]);
    }
    return 1;
}
FUNC(int)    rand_wchoice    (RNGSELF, wchoice_args_t *a, Py *r, size_t n) {

    Py o;
    rand64_t rv;
    size_t k, i;
    union { void *mem; double *dmem; size_t *imem; } mem;

    switch(a->bisect_as) {
        case 'd':
            mem.dmem = (double*)a->mem;
            goto D;
        case 'i':
            mem.imem = (size_t*)a->mem;
            goto N;
        default:
            goto X;
    }
    D:
    for(i=0; i < n; ++i) {
        rv.d = rand_DOUBLE();
        k = bisect_double(mem.dmem, rv.d * a->sum.d , a->hi);
        if(k >= a->hi)
            goto bindex;
        o = LIST_ITEMS(a->pop)[k];
        if(Choices_Check(o)) {
            r[i] = choices_next((ChoicesObject*)o);
            if(!r[i])
                return 0;
            continue;
        }
        r[i] = Py_NEWREF(o);
    }
    return 1;
    N:
    for(i=0; i < n; ++i) {
        rv.N = next_index(rng, &a->sum.c);
        k = bisect_size_t(a->mem, rv.N, a->hi);
        if(k >= a->hi)
            goto bindex;
        o = LIST_ITEMS(a->pop)[k];
        if(Choices_Check(o)) {
            r[i] = choices_next((ChoicesObject*)o);
            if(!r[i])
                return 0;
            continue;
        }
        r[i] = Py_NEWREF(o);
    }
    return 1;
    bindex:
    return PySystemError("bisect returned index >= length"), 0;
    X:
    return PySystemError("invalid wchoice wtype %s", a->bisect_as), 0;
}
FUNC(int)    rand_sample     (RNGSELF, sample_args *a, Py *elem, size_t k) {
    for(sizes_t j, i={0}; (i.u < k); ) {
        j.u = random_partial(rng, a->b);
        if(j.u >= pyseq_SIZE(a->sq) || set_add_entry(a->indices, &j) != 1)
            continue;
        elem[i.i++] = PySequence_item(a->sq, j.i);
    }
    return 1;
}
FUNC(double) normalvar       (RNGSELF, c_double mu, c_double sigma) {

    double u1=0., u2=0., z=0.0, zz=0.0;
    A:
    u1 = rand_DOUBLE();
    u2 = 1.0 - rand_DOUBLE();
    z = 1.7155277699214135 * (u1-0.5)/u2;
    zz = (z*z/4.0);
    if(-log(u2) < zz)
        goto A;
    return mu + z * sigma;
}
FUNC(double) gammavar        (RNGSELF, double alpha, double beta) {
    /* split this up someday...*/
    double u, u1, u2, b, p, x, v, z, r;
    if(alpha > 1.0) {
        double ainv = sqrt(2.0 * alpha - 1.0);
        double bbb = alpha - 1.3862943611198906;
        double ccc = alpha + ainv;
        while(1) {
            u1 = rand_DOUBLE();
            if(!((1e-7 < u1) && (u1 < .9999999)))
                continue;
            u2 = 1.0 - rand_DOUBLE();
            v = log(u1/(1.0-u1))/ainv;
            x = alpha*exp(v);
            z = u1*u1*u2;
            r = bbb+ccc*v-x;
            if(((r + 2.504077396776274 - 4.5*z) >= 0.0) || (r >= log(z)))
                return x * beta;
        }
    }
    if(alpha == 1.0) {
        u = rand_DOUBLE();
        while(u <= 1e-7)
            u = rand_DOUBLE();
        return -log(u) * beta;
    }
    while(1) {
        u = rand_DOUBLE();
        b = (2.718281828459045 + alpha)/2.718281828459045;
        p = b*u;
        if(p <= 1.0)
            x = pow(p, (1.0/alpha));
        else
            x = -log((b-p)/alpha);
        u1 = rand_DOUBLE();
        if((p > 1.0) && (u1 <=pow( x , (alpha - 1.0))))
            break;
        if( u1 <= exp(-x))
            break;
    }
    return x * beta;
}
FUNC(double) rand_beta       (RNGSELF, double alpha, double beta) {
    double const y = gammavar(rng, alpha, 1.0);
    if(!y)
        return 0.0;
    return y / (y+gammavar(rng, beta, 1.0));
}
FUNC(double) rand_vonmises   (RNGSELF, double mu, double kappa) {
    double u1, z, u2, d, u3, s, r, q, f;
    double theta = 0.0;
    int mod;

    if(kappa <= 1e-6)
        return TAU * rand_DOUBLE();

    s = 0.5 / kappa;
    r = s + sqrt(1.0 + s * s);

    while(1) {
        u1 = rand_DOUBLE();
        z = cos(PI * u1);

        d = z / (r + z);
        u2 = rand_DOUBLE();
        if((u2 < 1.0 - d * d) || (u2 <= ((1.0 - d) * exp(d))))
            break;
    }
    q = 1.0 / r;
    f = (q + z) / (1.0 + q * z);
    u3 = rand_DOUBLE();
    if(u3 > 0.5)
        mod = rand_fmod(mu + acos(f), TAU, &theta);
    else
        mod = rand_fmod(mu - acos(f), TAU, &theta);
    if(!mod)
        return PySystemError("div by zero"), 0.0;
    return theta;
}
FUNC(double) rand_triangular (RNGSELF, double lo, double hi, double c) {

    double u = rand_DOUBLE();
    if(u > c) {
        double t = lo;
        u = 1.0 - u;
        c = 1.0 - c;
        hi = ((lo=hi), t) ;
    }
    return (lo + (hi - lo) * sqrt(u*c));
}
FUNC(double) rand_weibull    (RNGSELF, statf_args_t *a) {
    return (a->alpha * pow(-log(1.0 - rand_DOUBLE()), 1.0/a->beta));
}
FUNC(double) rand_expo       (RNGSELF, c_double lambda) {
    return -log(1.0 - rand_DOUBLE())/lambda;
}
FUNC(double) rand_pareto     (RNGSELF, c_double alpha) {
    return 1.0 / pow(1.0 - rand_DOUBLE(), 1.0/alpha);
}
FUNC(double) rand_uniform    (RNGSELF, c_double a, c_double b) {
    return a + (b-a) * random_double(rng);
}
FUNC(void)   rand_shuffle    (RNGSELF, py_list *lst) {
#   define a lst->ob_item[i.i]
#   define b lst->ob_item[j.i]
    Py c;
    sizes_t i = {Py_SIZE(lst)}, j;
    A:
    if(i.i<2)
        return;
    j.u = next_index(rng, &i);
    --i.i;
    c = a; a=b; b=c;
    goto A;
#   undef a
#   undef  b
}
RAND_IMPL(sample)          (RNGSELF, sample_args *a) {
    Py result = __list__(a->k.i);
    if(!result)
        return NULL;
    if(a->sq) {
        if(!rand_sample(rng, a, LIST_ITEMS(result), a->k.u))
            Py_DECREF(result);
    }
    else
        if(!rand_wsample(rng, (void*)a, LIST_ITEMS(result), a->k.u))
            Py_DECREF(result);
    return result;
}
RAND_IMPL(choice)          (RNGSELF, choice_args_t *a) {
    Py result = NULL;
    rand_choice(rng, a, &result, 1);
    return result;
}
RAND_IMPL(rand_index3)     (RNGSELF, args_t(rand_index) *a) {
    /* switch to next_index for 64 bit builtds with b > 60 */
    size_t r = next_index(rng, &a->max);
    return py_int_from_size_t(r);
}
RAND_IMPL(rand_index)      (RNGSELF, args_t(rand_index) *a) {
    sizes_t r;
    do {
        r.u = (size_t)(random_next(rng) >> a->sh);
    } while(r.u >= a->max.u);
    return py_int_from_sizes_t(&r, 'N');
}
RAND_IMPL(betavariate)     (RNGSELF, args_t(betavariate) *a) {
    return __float__(rand_beta(rng, a->alpha, a->beta));
}
RAND_IMPL(expovariate)     (RNGSELF, args_t(expovariate) *a) {
    if(!a->lambda)
        return PyValueError("expovariate() lambda must be non-zero");
    return  __float__(rand_expo(rng, a->lambda));
}
RAND_IMPL(normalvariate)   (RNGSELF, args_t(normalvariate) *a) {
    return __float__(normalvar(rng, a->mu, a->sigma));
}
RAND_IMPL(lognormvariate)  (RNGSELF, args_t(normalvariate) *a) {
    return __float__(exp(normalvar(rng, a->mu, a->sigma)));
}
RAND_IMPL(triangular)      (RNGSELF, args_t(triangular) *a) {
    double r;
    if(a->idk)
        r = a->lo;
    else
        r = rand_triangular(rng, a->lo, a->hi, a->c);
    return __float__(r);
}
RAND_IMPL(vonmisesvariate) (RNGSELF, args_t(vonmisesvariate) *a) {
    return __float__(rand_vonmises(rng, a->mu, a->kappa));
}
RAND_IMPL(weibullvariate)  (RNGSELF, statf_args_t *a) {
    if(!a->beta)
        return PyValueError("weibullvariate() beta must be non-zero");
    return __float__(rand_weibull(rng, a));
}
RAND_IMPL(gammavariate)    (RNGSELF, args_t(gammavariate) *a) {
    if(a->alpha <= 0.0 || a->beta <= 0.0)
        return PyValueError("gammavariate() alpha and beta must be > 0.0");
    return __float__(gammavar(rng, a->alpha, a->beta));
}
RAND_IMPL(paretovariate)   (RNGSELF, args_t(paretovariate) *args) {
    if(!args->alpha)
        return PyValueError("paretovariate() alpha must be non-zero");
    return __float__(rand_pareto(rng, args->alpha));
}
RAND_IMPL(uniform)         (RNGSELF, args_t(uniform) *a) {
    return __float__(rand_uniform(rng, a->a, a->b));
}
RAND_IMPL(wchoice)         (RNGSELF, wchoice_args_t *a) {
    Py result = NULL;
    rand_wchoice(rng, a, &result, 1);
    return result;
}
RAND_IMPL(randint_fast)    (RNGSELF, randint_args_t *args) {
    rand64_t r;
    do {
        r.u = random_next(rng) >> (args->sh);
    } while(r.u > (args->width).u);
    r.i += args->base.i;
    return py_int_from_rand64_t(r);
}
RAND_IMPL(randint_slow)    (RNGSELF, brandint_args_t *args ) {
    Py t, result;
    if(!args->mem)
        return PyErr_NoMemory();
    size_t const len = (size_t)Py_SIZE(args->big_width);
    size_t const msd = len - 1;
    do {
        random_digits(rng, args->mem, len);
        args->mem[msd] >>= args->sh;
    } while(cmp_gt(args->mem, ((py_int*)args->big_width)->ob_digit, len));
    if(!(t = py_int_from_digits(args->mem, len)))
        return NULL;
    if(Py_SIZE(args->big_base)) {
        result = py_int_add(args->big_base, t);
        Py_DECREF(t);
    }
    else
        result = t;
    return result;
}
RAND_IMPL(randint_zero)    (RNGSELF, void *args ) {
    return Py_NEWREF(SMALL_INTS[0]);
}
FUNC(Py) statf_unpackers(randiter *ri, randiter_args_t *t, Py args);
#define PY_METHOD(f) static PyObject *(f)
FUNC(Py) choices_repr   (CHOICESSELF) {
    const char *name;
    Py t;
    if(ro->next_n == rand_wchoice)
        return __format__("<choices object from a weighted population>");

    t = pyseq_SQ(&Choices_RC_ARGS(ro)->sq);
    name = Py_TYPE(t)->tp_name;
    return __format__("<choices object from a %s-based population>", name);
}
FUNC(Py) choices_next   (CHOICESSELF) {
    Object r = NULL;
    ro->next_n(ro->rng, ro->ROLLER_ARGFIELD, &r, 1);
    return r;
}
FUNC(Py) choices_next_n (CHOICESSELF, Py arg) {
    Object r;
    sizes_t n;
    r  = parse_next_n((RandRoller*)ro, arg, &n);
    if(!r)
        return r;
    if(!ro->next_n(ro->rng, ro->ROLLER_ARGFIELD, LIST_ITEMS(r), n.u))
        Py_DECREF(r);
    return r;
}
FUNC(Py) choices_sample (CHOICESSELF, Py arg) {

    Py r;
    sample_args a;
    choice_arg_cast_t ca = {ro->args.any};
    sizes_t *pop_size;

    if(!PyLong_Check(arg))
        return PyTypeError("k must be an integer");
    if(!py_int_as_sizes_t((py_int*)arg, &a.k, 'N'))
        return NULL;

    int const v = ro->next_n == rand_wchoice;
    if(v) {
        a.wc = ca.wc;
        pop_size = &a.wc->sum.c;
    }
    else {
        a.sq = &ca.rc->sq;
        pop_size = pyseq_SIZES(a.sq);
    }
    if(!check_sample_k(&a, pop_size))
        return NULL;
    if(!(r = __list__(a.k.i)))
        return NULL;
    if(!(a.indices = new_set(&a.k))) {
        Py_DECREF(r);
        return NULL;
    }
    if(v) {
        if(!rand_wsample(ro->rng, (wsample_args*)&a, LIST_ITEMS(r), a.k.u))
            Py_DECREF(r);
    }
    else {
        if(!rand_sample(ro->rng, &a, LIST_ITEMS(r), a.k.u))
            Py_DECREF(r);
    }
    PyObject_Free(a.indices);
    return r;
}

FUNC(Py) choices_size    (CHOICESSELF) {
    if(ro->next_n == rand_wchoice)
        switch(Choices_WC_TYPE(ro)) {
            case 'd': return __float__(Choices_WSUM_D(ro));
            case 'i': return py_int_from_size_t(Choices_WSUM_I(ro));
            default: return PySystemError("choices object has invalid wc type");
        }
    return py_int_from_size_t(pyseq_SIZE(&Choices_RC_ARGS(ro)->sq));
}
FUNC(Py) choices_population    (CHOICESSELF) {
    Py  r;
    choice_arg_cast_t ca = {ro->args.any};
    sample_args a;

    if(ro->next_n == rand_wchoice) {
        a.wc = ca.wc;
        r = __list__(Py_SIZE(a.wc->pop));
        if(!r)
            return NULL; 
        switch(a.wc->bisect_as) {
            case 'i':
                ssize_t *wi = (ssize_t*)a.wc->mem;//weights;
                for(ssize_t l=0; l<Py_SIZE(a.wc->pop); ++l)
                    if(!(LIST_ITEMS(r)[l] = py_int_from_ssize_t(wi[l])))
                        return NULL;
                break;
            case 'd':
                double *wd = (double*)a.wc->mem;//weights;
                for(ssize_t l=0; l<Py_SIZE(a.wc->pop); ++l)
                    if(!(LIST_ITEMS(r)[l] = __float__(wd[l])))
                        return NULL;
                break;
            default:
                Py_DECREF(r);
                return PySystemError("invalid bisect_as field");
            }
        return pbv("ON", a.wc->pop, r);
    }
    return Py_NEWREF(pyseq_SQ(&ca.rc->sq));
}
VOID_FUNC(sample_destr)    (sample_args_t *a) {
    PySeq_CLEAR(&a->sq);
    PyObject_Free(a->indices);
}
VOID_FUNC(wchoice_destr)   (CHOICESSELF) {
    Py_XDECREF   (ro->args.wc->pop);
    PyObject_Free(ro->args.wc->mem);
}
VOID_FUNC(schoice_destr)   (CHOICESSELF) {
    Py_DECREF(ro->args.rc->sq.base.sq);
}
VOID_FUNC(choices_dealloc) (CHOICESSELF) {
    Py_XDECREF(ro->rng);
    if(ro->destr)
        ro->destr(ro);
    PyObject_Free(ro->args.any);
    PyObject_Del(ro);
}
VOID_FUNC(brandint_destr)  (DiceObject *ro) {
    Py_DECREF(ro->args.slow->big_base);
    Py_DECREF(ro->args.slow->big_width);
    PyMem_FREE(ro->args.slow->mem);
}
VOID_FUNC(randint_destr)   (DiceObject *ro) {
    return;
}
VOID_FUNC(dice_dealloc)    (DiceObject* ro) {
    if(ro->rng) Py_DECREF(ro->rng);
    if(ro->destr) ro->destr(ro);
    if(ro->args.any) PyObject_Free(ro->args.any);
    PyObject_Del(ro);
}
FUNC(Py)  dice_calc_hi_lo(DiceObject *ro, Py *lo) {

    rand64_t fast;

    if(!ro->generator) {
        *lo = Py_NEWREF(SMALL_INTS[0]);
        return Py_NEWREF(SMALL_INTS[0]);
    }
    if(ro->generator == 'f') {
        *lo = py_int_from_rand64_t(ro->args.fast->base);
        fast.i = ro->args.fast->width.i + ro->args.fast->base.i;
        return py_int_from_rand64_t(fast);
    }
    if(ro->generator != 's')
        return (*lo=PySystemError("invalid dice generator flag"));
    *lo = Py_NEWREF(ro->args.slow->big_base);
    return PyNumber_Add(ro->args.slow->big_base, ro->args.slow->big_width);
}
FUNC(Py) dice_max(DiceObject *ro) {
    Py lo=NULL, hi, r;
    
    if(!(hi = dice_calc_hi_lo(ro, &lo)))
        r = NULL;
    else 
        r = hi;
    Py_XDECREF(lo);
    return r;
}
FUNC(Py) dice_min(DiceObject *ro) {
    Py lo=NULL, hi, r;
    
    if(!(hi = dice_calc_hi_lo(ro, &lo)))
        r = NULL;
    else
        r = lo;
    Py_XDECREF(hi);
    return r;
}
FUNC(Py)  dice_repr(Py self) {
    DiceObject *ro = (DiceObject*)self;
    Py lo, hi, r=NULL, v=NULL;

    hi = dice_calc_hi_lo(ro, &lo);
    if(hi==NULL || lo==NULL)
        goto end;
    v = pbv("OO", lo, hi);
    r = PyUnicode_FromFormat("dice%R", v);
    Py_DECREF(v);
    end:

    Py_XDECREF(lo);
    Py_XDECREF(hi);

    return r;
}
FUNC(int) dice_next_zero(RNGSELF, void *a, Py *list, size_t n) {
    for(size_t i=0; i<n; ++i)
        list[i] = Py_NEWREF(SMALL_INTS[0]);
    return 1;
}
FUNC(int) dice_next_fast(RNGSELF,  randint_args_t *a, Py *list, size_t n) {
    for(size_t i=0; i<n; ++i)
        if(!(list[i] = random_randint_fast_impl(rng, a)))
            return 0;;
    return 1;
}
FUNC(int) dice_next_slow(RNGSELF, brandint_args_t *a, Py *list, size_t n) {
    for(size_t i=0; i<n; ++i)
        if(!(list[i] = random_randint_slow_impl(rng, a)))
            return 0;
    return 1;
}
FUNC(Py)  dice_next    (DiceObject* ro) {
    Object r = NULL;
    ro->next_n(ro->rng, ro->ROLLER_ARGFIELD, &r, 1);
    return r;
}
FUNC(Py)  dice_next_n  (DiceObject* ro, Py arg) {
    Object r;
    sizes_t n;
    r  = parse_next_n((RandRoller*)ro, arg, &n);
    if(!r)
        return r;
    if(!ro->next_n(ro->rng, ro->ROLLER_ARGFIELD, LIST_ITEMS(r), n.u))
        Py_DECREF(r);
    return r;
}
RAND_METH(select)           (RNGSELF, Py arg)  {

    Py result;
    choice_arg_cast_t a;

    char buf [CHOICEARGS_BUF] = {0};
    a.buffer = buf;
    int const r = parse_choice_args(arg, &a);

    switch(r) {
        case 0:
            if(!PyErr_Occurred())
                return PySystemError("failed to set error");
            return NULL;
        case 'm':
            result = random_wchoice_impl(rng, a.wc);
            wchoice_destr2(&a);
            break;
        case 'r':
            result = random_choice_impl(rng, a.rc);
            PySeq_CLEAR(&a.rc->sq);
            break;
        default:
            return PySystemError("parse_choice_args invalid flag '%d'",r);
    }
    return result;
}
RAND_METH(choices)          (RNGSELF, Py arg)  {

    ChoicesObject *ro;

    ro = __object__(&Choices_Type);
    if(!ro)
        return NULL;
    ro->args.any = PyObject_MALLOC(CHOICEARGS_BUF);
    ro->ROLLER_ARGFIELD = ro->args.any;
    int const r = parse_choice_args(arg, &ro->args);
    if(!r) {
        if(!PyErr_Occurred())
            return PySystemError("parse choice args fail w/o error");
        Py_DECREF(ro);
        return NULL;
    }
    Py_NEWREF(ro->rng = rng);
    switch(r) {
        case 'm':
            ro->next_n     = rand_wchoice;
            ro->destr      = wchoice_destr;
            break;
        case 'r':
            ro->next_n = rand_choice;
            ro->destr  = schoice_destr;
            break;
        default:
            Py_XDECREF(ro);
            return PySystemError("parse_choice_arg returned invalid code");
    }
    return (Object)ro;
}
RAND_METH(dice)             (RNGSELF, Py args) {

    Py result;
    DiceObject *ro = __object__(&Dice_Type);

    if(!ro)
        return NULL;
    ro->ROLLER_ARGFIELD = PyObject_MALLOC(RANDINT_ARGS_BUF);
    ro->args.any = ro->ROLLER_ARGFIELD;
    Py_NEWREF(ro->rng = rng);
    if(!ro->ROLLER_ARGFIELD)
        PyErr_NoMemory();
    ro->generator = parse_dice_args(&ro->args, args);
    if(PyErr_Occurred())
        goto fail;

    switch(ro->generator) {
        case 'z':
            ro->next_n = dice_next_zero;
            ro->destr  = randint_destr;
            break;
        case 'f':
            ro->next_n = dice_next_fast;
            ro->destr  = randint_destr;
            break;
        case 's':
            ro->next_n = dice_next_slow;
            ro->destr = brandint_destr;
            break;
        default:
            result = PySystemError("parse_dice_arg invalid r value");
            goto fail;
    }
    return (Py)ro;
  fail:
    Py_DECREF(ro);
    return NULL;
}
#endif
#if !defined RandIter_Flip_Impl
FUNC(Py)  flip_iter_unpack_args(randiter *ri, randiter_args_t *t, Py args) {

    DEFINE_VARARGS(flips, 1, 'o', 'd', 'o', 'o') = {"n", "odds", "h", "t"};
    VARARG(Py, count, NULL);
    PVARARG(t->flip.odds);
    PVARARG(t->flip.heads);
    PVARARG(t->flip.tails);
    PARSE_VARARGS();

    if(t->flip.odds >= 1.0 || t->flip.odds < 0.0)
        return PyValueError("odds for heads should be between [0.0, 1.0)");
    Py_INCREF(t->flip.heads);
    Py_INCREF(t->flip.tails);
    return count;
}
FUNC(Py)  randiter_flip_next(randiter *ri) {
    if(random_double(randiter_RNG(ri)) < randiter_FLIP_ODDS(ri))
        return Py_NEWREF(randiter_FLIP_HEADS(ri));
    return Py_NEWREF(randiter_FLIP_TAILS(ri));
}
FUNC(Py)  randiter_flip_repr(randiter *ri) {

    Py result, heads, tails;

    SETUP_DOUBLE_BUF(odds_buf);

    sizes_t const count = {randiter_COUNTER(ri)};
    char *repr_buf      = NULL;
    double const odds   = randiter_FLIP_ODDS(ri);

    if(double_repr(randiter_FLIP_ODDS(ri), odds_buf) < 0)
        return NULL;

    heads = randiter_FLIP_HEADS(ri);
    tails = randiter_FLIP_TAILS(ri);

    if(count.i < 0) {
        if(call_repr(&repr_buf, "flips", "None, %s, %R, %R") < 0)
            return NULL;
        result = __format__(repr_buf, odds_buf, heads, tails);
    }
    else {
        SETUP_SIZES_BUF(sizes_buf);
        if(sizes_repr(&count, sizes_buf, 'i') < 0)
            return NULL;
        if(call_repr(&repr_buf, "flips", "%s, %s, %R, %R") < 0)
            return NULL;
        result = __format__(repr_buf, sizes_buf, odds_buf, heads, tails);
    }
    PyMem_Free(repr_buf);
    return result;
}
FUNC(void)randiter_flip_destr (randiter *ri) {
    Py_DECREF(randiter_FLIP_HEADS(ri));
    Py_DECREF(randiter_FLIP_TAILS(ri));
}
#endif
#if !defined RandIter_MethodImplementatinons
FUNC(int) randiter_parse_count(ssize_t *count, Py arg) {

    sizes_t n;
    Py integral;

    if(!arg)
        return 1;
    if(arg==Py_None) {
        *count = -1;
        return 0;
    }
    integral = py_index(arg);
    if(!integral)
        return -1;
    int const r = py_int_as_sizes_t((py_int*)integral, &n, 'n');
    Py_DECREF(integral);
    if(!r)
        return -1;
    *count = n.i < 1? 0: n.i;
    return 0;
}
FUNC(Py)  randiter_take(randiter *ri) {
    return ri->next(ri);
}
FUNC(Py)  randiter_next(randiter *ri) {
    if(!randiter_COUNTER(ri))
        return NULL;
    if(randiter_COUNTER(ri) > 0)
        --randiter_COUNTER(ri);
    return ri->next(ri);
}
FUNC(Py)  randiter_repr(randiter *ri) {
    return randiter_CALL_REPR(ri);
}
FUNC(void)randiter_dealloc  (randiter *ri) {
    Py_DECREF(ri->rng);
    if(randiter_DESTR(ri))
        randiter_CALL_DESTR(ri);
    PyObject_Del(ri);
}
FUNC(Py)  randiter_iter(randiter *ri){
    return Py_NEWREF(ri);
}
#endif
FUNC(void*) randiter_new(RNGSELF, randiter_setup_args *x) {
    Py cnt;
    randiter *ri;

    ri = __object__(&RandIter_Type);
    if(!ri)
        return NULL;
    (Py)randiter_RNG(ri) = Py_NEWREF(rng);
    cnt = x->unpacker(ri, &x->defaults, x->args);
    memcpy(ri->buf, &x->defaults, RANDITER_BUF_SIZE);
    if(randiter_parse_count(&ri->counter, cnt))
        goto X;

    ri->next =  x->funcs->next;
    ri->repr =  x->funcs->repr;
    ri->destr = x->funcs->destr;

    return ri;
  X:
    Py_DECREF(ri);
    return NULL;
}
FUNC(Py)  randiter_stat_repr(randiter *r) {
    prn;
}
#include "rv_autorepr.h"
FUNC(Py) randiter_rv_next(randiter *ri) {
    #define call_impl2(func, args) \
        random_##func##_impl(rng, (void*)&_riter_stat_args_(ri).args)
    #define call_impl(func) \
        return random_##func##_impl(rng, (void*)&_riter_stat_args_(ri).beta)
    RandomObject *rng = randiter_RNG(ri);
    RV_FLAG const flag = randiter_STAT_FLAG(ri);
    switch(flag){
        case RV_BETA:     call_impl(betavariate);
        case RV_EXPO:     call_impl(expovariate);
        case RV_GAMMA:    call_impl(gammavariate);
        case RV_LOGNORM:  call_impl(lognormvariate);
        case RV_NORM:     call_impl(normalvariate);
        case RV_PARETO:   call_impl(paretovariate);
        case RV_TRI:      call_impl(triangular);
        case RV_UNI:      call_impl(uniform);
        case RV_VONMISES: call_impl(vonmisesvariate);
        case RV_WEIBULL:  call_impl(weibullvariate);
        default: prn;
    }
    #undef call_impl
    #undef call_impl2
}
RAND_METH(flips) (RNGSELF, Py args) {

    static const randiter_funcs flip_funcs = {
        randiter_flip_next,
        (reprfunc)randiter_flip_repr,
        randiter_flip_destr
    };
    randiter_setup_args a = {
        .args     = args,
        .defaults = {Py_True, Py_False, 0.5},
        .funcs    = &flip_funcs,
        .unpacker = flip_iter_unpack_args,
    };
    return (Py)randiter_new(rng, &a);
}
#ifdef INCLUDE_SETSTATE
FUNC(Py) random_set_state(Py m, Py args) {
    Py rng;
    uint64_t s0, s1;

    if(Py_SIZE(args) != 3)
        return PyTypeError("set_state requires 3 arguments: rng, s0, s1");
    rng = TUPLE_ITEMS(args)[0];
    if(!PyObject_IsInstance(rng, (Py)&Random_Type))
        return PyTypeError("set_state requires Random object");
    s0 = PyLong_AsUnsignedLongLong(TUPLE_ITEMS(args)[1]);
    s1 = PyLong_AsUnsignedLongLong(TUPLE_ITEMS(args)[2]);
    if(ERROR())
        return NULL;
    rng_STATE()[0] = s0;
    rng_STATE()[1] = s1;
    prn;
}
#endif
RAND_METH(flip)             (RNGSELF, Py arg)  {

    Py r;

    if(!Random_PRAND_RBIT(rng))
        prand_renew(rng);
    ssize_t const t = (--Random_PRAND_RBIT(rng), Random_PRAND_MSB(rng));
    r = t? arg: Py_False;
    rng_PRAND_UVAL() <<= 1;
    return Py_NEWREF(r);
}
RAND_METH(rand_bits)        (RNGSELF, Py arg)  {

    digit *p;
    Py result;
    sizes_t b;

    if(!(PyLong_Check(arg)))
        return PyTypeError("number of bits must be positive integer > 1");
    if(!py_int_as_sizes_t(((py_int*)arg), &b, 'N'))
        return NULL;
    if(b.i < 61) {
        rand64_t r = {.u = random_next(rng) >> (64-b.u)};
        return py_int_from_rand64_t(r);
    }

    sizes_t const n = {(b.u + PyLong_SHIFT -1) / PyLong_SHIFT};

    p = PyMem_Malloc(n.u * sizeof(digit));
    if(!p)
        return PyErr_NoMemory();
    random_digits(rng, p, n.u);
    p[n.u-1] >>= n.u * PyLong_SHIFT - b.u;
    result = py_int_from_digits(p, n.i);
    PyMem_Free(p);
    return result;
}
RAND_METH(jump)             (RNGSELF)          {

    if(!random_jump(rng))
        return NULL;
    Py_RETURN_NONE;
}
RAND_METH(rand_bytes)       (RNGSELF, Py args) {
        
    py_alias_t r;
    Py_buffer view;
    DEFINE_VARARGS(rand_bytes, 1, 'n', 'o') = {"size", "buf"};
    VARARG(rand64_t, size, {-1});
    VARARG(Py , buf, NULL);
    PARSE_VARARGS();

    if(size.n<1)
        return PyTypeError("number of bytes must be positive integer > 1");
    if(buf==NULL) {
        if(!(r.ob=PyBytes_FromStringAndSize(NULL, size.n)))
            return NULL;
        random_bytes(rng, r.bytes->ob_sval, size.N);
        return r.ob;
    }
    if(PyObject_GetBuffer(buf, &view, PyBUF_SIMPLE|PyBUF_C_CONTIGUOUS))
        return NULL;
    if(view.readonly) {
        r.ob = PyTypeError("'%s' object is read-only", Py_TP_NAME(buf));
        goto end;
    }
    if(size.n > view.len) {
        r.any =PyValueError("number of bytes requested will not fit in "
                            "the %s object's buffer", Py_TP_NAME(buf));
        goto end;
    }
    else
        r.ob = Py_NEWREF(buf);
    random_bytes(rng, (uint8_t*)view.buf, size.N);
  end:
    PyBuffer_Release(&view);
    return r.ob;
}
RAND_METH(rand_index)       (RNGSELF, Py arg)  {
    args_t(rand_index) r;
    if(!PyLong_Check(arg))
        return SIMPLE_TYPE_ERROR(arg, "an integer is required");
    if(!(py_int_as_sizes_t((py_int*)arg, (sizes_t*)&r, 'N')))
        return NULL;

    uint8_t const b = size_t_bit_length(r.max);
    if(!b)
        return PyValueError("cannot choose from an empty range");
    #ifndef RAND32
    if(b > 60) {
        r.pb = b;
        return random_rand_index3_impl(rng, &r);
    }
#   endif
    r.sh = 64 - (r.pb=b);
    return random_rand_index_impl(rng, &r);
}
RAND_METH(rand_int)         (RNGSELF, Py args) {
    Py result;
    dice_arg_cast_t r;
    char buf[RANDINT_ARGS_BUF] = {0};

    r.buffer = buf;
    int const v = parse_dice_args(&r, args);

    switch(v) {
        case 'f':
            return random_randint_fast_impl(rng, r.fast);
        case 's':
            result = random_randint_slow_impl(rng, r.slow);
            PyMem_Free(r.slow->mem);
            Py_DECREF(r.slow->big_base);
            Py_DECREF(r.slow->big_width);
            return result;
        case -1:
            return NULL;
        case 'z':
            return Py_NEWREF(SMALL_INTS[0]);
    }
    return PySystemError("parse_dice_arg invalid r value(%i)",v);;
}
RAND_METH(rand_float)       (RNGSELF)          {
    return __float__(rand_DOUBLE());
}
RAND_METH(sample)           (RNGSELF, Py args) {

    auto Py r;
    sample_args a = {0};

    DEFINE_VARARGS(sample, 2, 'o', 'n') = {"pop", "k"};
    VARARG(Py, pop, NULL);
    PVARARG(a.k);
    PARSE_VARARGS2(0);

    int const v = parse_sample_args(pop, &a);

    switch(v) {
        case 0:
            return NULL;
        case 'r':
            if(a.wc)
                return NULL;
            r = random_sample_impl(rng, &a);
            PySeq_CLEAR(a.sq);
            break;
        case 'w':
            r = random_sample_impl(rng, &a);
            Py_XDECREF(a.t.wc.pop);
            PyObject_Free(a.t.wc.mem);
            break;
        default:
            return PySystemError("bad sample flag %d", v);
    }
    PyObject_Free(a.indices);
    return r;
}
RAND_METH(shuffle)          (RNGSELF, Py arg)  {
    if(!PyList_Check(arg))
        return PyTypeError("shuffle() requires a list");
    if(Py_SIZE(arg)<1)
        return PyValueError("cannot shuffle empty list");
    rand_shuffle(rng, (py_list*)arg);
    Py_RETURN_NONE;
}
RAND_METH(shuffled)         (RNGSELF, Py arg)  {
    PySequence sq = PySequence_new();
    shuffled_args_t a = {.sq=&sq, .mem=NULL};
    if(!py_sequence_parse(arg, a.sq, TRUE))
        return NULL;
    if(!pyseq_SSIZE(a.sq)) {
        a.mem = PyValueError("cannot shuffle an empty sequence");
        goto end;
    }
    if(!(a.mem = __list__(pyseq_SSIZE(a.sq))))
        goto end;
    for(ssize_t i=0; i<Py_SIZE(a.mem); ++i)
        if(!(_PyList_ITEMS(a.mem)[i] = PySequence_item(a.sq, i)))
            goto end;
    rand_shuffle(rng, (py_list*)a.mem);
  end:
    if(ERROR())
        Py_XDECREF(a.mem);
    PySeq_CLEAR(a.sq);
    return a.mem;
}
RAND_METH(split)            (RNGSELF, Py args) {
    
    py_alias_t sub, r;
    
    DEFINE_VARARGS(split, 1, 'n', 'n') = {"n", "w"};
    VARARG(rand64_t, n, {.n=0});
    VARARG(rand64_t, w, {.n=1});
    PARSE_VARARGS();
    
    if(n.n<0)
        return PyValueError("number of substreams must be >= 0");
    if(w.n<1)
        return PyValueError("invalid number of jumps per substream");
    if(!n.n)
        return __list__(0);
    if(!(r.ob = __list__(n.n)))
        return NULL;
    for(ssize_t i=0; i<n.n; ++i) {
        if(!(sub.any  = random_copy(rng, Py_TYPE(rng))))
            goto fail;
        rng = (RandomObject*)(r.list->ob_item[i] = sub.ob);
        for(ssize_t j=0; j<w.n; ++j)
            if(!random_jump(sub.any))
                goto fail;
    }
    return r.ob;
  fail:
    Py_XDECREF(r.ob);
    return NULL;
}

FUNC(Py)  beta_iter_unpack(betavariate_args_t *a, Py args) {

    DEFINE_VARARGS(iter_beta, 3, 'o', 'd', 'd') = {"n", "alpha", "beta"};
    VARARG(Py, count, NULL);
    PVARARG(a->alpha);
    PVARARG(a->beta);
    PARSE_VARARGS();
    return count;
}
RAND_METH(iter_beta) (RNGSELF, Py args) {
    RV_ITER_SETUP(beta, RV_BETA, 0);
}
RAND_METH(rv_beta)      (RNGSELF, Py args) {
    IMPL_ARGS(a, betavariate);
    DEFINE_VARARGS(betavariate, 2, 'd', 'd') = {"alpha", "beta"};
    PVARARG(a.alpha);
    PVARARG(a.beta);
    PARSE_VARARGS();

    return random_betavariate_impl(rng, &a);
}
FUNC(Py) expo_iter_unpack(expovariate_args_t *a, Py args) {
    DEFINE_VARARGS(iter_expo, 2, 'o', 'd') = {"n", "lambda"};
    VARARG(Py, count, NULL);
    PVARARG(a->lambda);
    PARSE_VARARGS();
    return count;
}
RAND_METH(iter_expo) (RNGSELF, Py args) {
    RV_ITER_SETUP(expo, RV_EXPO, 0);
}
RAND_METH(rv_expo)      (RNGSELF, Py arg)  {
    IMPL_ARGS(a, expovariate);
    if(!py_number_as_double(arg, &a.lambda))
        return NULL;
    return random_expovariate_impl(rng, &a);
}
FUNC(Py) gamma_iter_unpack(gammavariate_args_t *a, Py args) {
    
    DEFINE_VARARGS(iter_gamma, 3, 'o', 'd', 'd') = {"n", "alpha", "beta"};
    VARARG(Py, count, NULL);
    PVARARG(a->alpha);
    PVARARG(a->beta);
    PARSE_VARARGS();
    return count;

}
RAND_METH(iter_gamma)(RNGSELF, Py args) {
    RV_ITER_SETUP(gamma, RV_GAMMA, 0.0);
}
RAND_METH(rv_gamma)         (RNGSELF, Py args) {
    IMPL_ARGS(a, gammavariate);
    DEFINE_VARARGS(gammavariate, 2, 'd', 'd') = {"alpha", "beta"};
    PVARARG(a.alpha);
    PVARARG(a.beta);
    PARSE_VARARGS();

    return random_gammavariate_impl(rng, &a);
}
FUNC(Py) lognorm_iter_unpack(normalvariate_args_t *a, Py args) {
    DEFINE_VARARGS(iter_log_normal, 3, 'o', 'd', 'd') = {"n", "mu", "sigma"};
    VARARG(Py, count, NULL);
    PVARARG(a->mu);
    PVARARG(a->sigma);
    PARSE_VARARGS();

    return count;
}
RAND_METH(iter_log_normal) (RNGSELF, Py args) {
    RV_ITER_SETUP(lognorm,  RV_LOGNORM, 0.0);
}
RAND_METH(rv_log_normal)    (RNGSELF, Py args) {
    normalvariate_args_t a = {0.0};

    DEFINE_VARARGS(rv_log_normal, 2, 'd', 'd') = {"mu", "sigma"};
    PVARARG(a.mu);
    PVARARG(a.sigma);
    PARSE_VARARGS();

    return random_lognormvariate_impl(rng, &a);
}
FUNC(Py) normal_iter_unpack(normalvariate_args_t *a, Py args) {
    DEFINE_VARARGS(iter_normal, 3, 'o', 'd', 'd') = {"n", "mu", "sigma"};
    VARARG(Py, count, NULL);
    PVARARG(a->mu);
    PVARARG(a->sigma);
    PARSE_VARARGS();

    return count;
}
RAND_METH(iter_normal) (RNGSELF, Py args) {
    RV_ITER_SETUP(norm, RV_NORM, 0);
}
RAND_METH(rv_normal)        (RNGSELF, Py args) {
    normalvariate_args_t a = {0.0};

    DEFINE_VARARGS(rv_normal, 2, 'd', 'd') = {"mu", "sigma"};
    PVARARG(a.mu);
    PVARARG(a.sigma);
    PARSE_VARARGS();

    return random_normalvariate_impl(rng, &a);
}
FUNC(Py) pareto_iter_unpack(paretovariate_args_t *a, Py args) {
    DEFINE_VARARGS(iter_pareto, 2, 'o', 'd') = {"n", "alpha"};
    VARARG(Py, count, NULL);
    PVARARG(a->alpha);
    PARSE_VARARGS();
    return count;
}
RAND_METH(iter_pareto) (RNGSELF, Py args) {
    RV_ITER_SETUP(pareto, RV_PARETO, 0.0);
}
RAND_METH(rv_pareto)        (RNGSELF, Py arg)  {
    IMPL_ARGS(a, paretovariate);
    if(!py_number_as_double(arg, &a.alpha))
        return NULL;
    return random_paretovariate_impl(rng, &a);
}
FUNC(Py) triangular_iter_unpack(triangular_args_t *a, Py args) {
    a->lo=0.0, a->hi=1.0, a->c=0.0, a->idk=0;
    DEFINE_VARARGS(iter_triangular, 1, 'o', 'd', 'd', 'o') = {
        "n", "lo", "hi", "mode"};
    VARARG(Py, count, NULL);
    PVARARG(a->lo);
    PVARARG(a->hi);
    VARARG(Py , mode, NULL);
    PARSE_VARARGS();
    
    a->idk = (a->hi == a->lo);
    if(!a->idk) {
        if(mode != NULL) {
            double c;
            if(!py_number_as_double(mode, &c))
                return NULL;
            a->c = (c - a->lo) / (a->hi - a->lo);
        }
        else
            a->c = 0.5;
    }
    return count;
}
RAND_METH(iter_triangular) (RNGSELF, Py args) {
    RV_ITER_SETUP(triangular, RV_TRI, 0.0);
}
RAND_METH(rv_triangular)    (RNGSELF, Py args) {

    triangular_args_t a = {0.0, 1.0, 0.0};

    DEFINE_VARARGS(rv_triangular, 0, 'd', 'd', 'o') = {"lo", "hi", "mode"};
    PVARARG(a.lo);
    PVARARG(a.hi);
    VARARG(Py , mode, NULL);
    PARSE_VARARGS();
    
    a.idk = (a.hi == a.lo);
    if(!a.idk) {
        if(mode != NULL) {
            double c;
            if(!py_number_as_double(mode, &c))
                return NULL;
            a.c = (c - a.lo) / (a.hi - a.lo);
        }
        else
            a.c = 0.5;
    }
    
    return random_triangular_impl(rng, &a);
}
FUNC(Py) uniform_iter_unpack(uniform_args_t *a, Py args) {
    DEFINE_VARARGS(iter_uniform, 3, 'o', 'd', 'd') = {"n", "a", "b"};
    VARARG(Py, count, NULL);
    PVARARG(a->a);
    PVARARG(a->b);
    PARSE_VARARGS();
    return count;
}
RAND_METH(iter_uniform) (RNGSELF, Py args) {
    RV_ITER_SETUP(uniform, RV_UNI, 0.0);
}
RAND_METH(rv_uniform)       (RNGSELF, Py args) {
    args_t(uniform) a = {.b=0.0};
    DEFINE_VARARGS(rv_uniform, 1, 'd', 'd') = {"a", "b"};
    PVARARG(a.a);
    PVARARG(a.b);
    PARSE_VARARGS();
    return random_uniform_impl(rng, &a);
}
FUNC(Py) vonmises_iter_unpack(vonmisesvariate_args_t *a, Py args) {
    DEFINE_VARARGS(iter_vonmises, 3, 'o', 'd', 'd') = {"n", "mu", "kappa"};
    VARARG(Py, count, NULL);
    PVARARG(a->mu);
    PVARARG(a->kappa);
    PARSE_VARARGS();
    return count;
}
RAND_METH(iter_vonmises) (RNGSELF, Py args) {
    RV_ITER_SETUP(vonmises, RV_VONMISES, 0.0);
}
RAND_METH(rv_vonmises)      (RNGSELF, Py args) {
    IMPL_ARGS(a, vonmisesvariate);
    DEFINE_VARARGS(rv_vonmisesvariate, 2, 'd', 'd') = {"mu", "kappa"};
    PVARARG(a.mu);
    PVARARG(a.kappa);
    PARSE_VARARGS();

    return random_vonmisesvariate_impl(rng, &a);
}
FUNC(Py) weibull_iter_unpack(weibullvariate_args_t *a, Py args) {
    DEFINE_VARARGS(iter_weibull, 3, 'o', 'd', 'd') = {"n", "alpha", "beta"};
    VARARG(Py, count, NULL);
    PVARARG(a->alpha);
    PVARARG(a->beta);
    PARSE_VARARGS();
    return count;
}
RAND_METH(iter_weibull) (RNGSELF, Py args) {
    RV_ITER_SETUP(weibull, RV_WEIBULL, 0.0);
}
RAND_METH(rv_weibull)       (RNGSELF, Py args) {
    IMPL_ARGS(a, weibullvariate);

    DEFINE_VARARGS(rv_weibullvariate, 2, 'd', 'd') = {"alpha", "beta"};
    PVARARG(a.alpha);
    PVARARG(a.beta);
    PARSE_VARARGS();
    statf_args_t ar = {.alpha = a.alpha, .beta=a.beta};
    return random_weibullvariate_impl(rng, &ar);
}
RAND_METH(__copy__)         (RNGSELF) {
    return random_copy(rng, Py_TYPE(rng));
}
RAND_METH(__getnewargs_ex__)(RNGSELF) {
    Py state = PyBytes_FromStringAndSize(NULL, RANDOM_SIZE);
    size_t rb = rng_PRAND_RBIT();
    size_t prand = rng_PRAND_UVAL();
    if(!state)
        return NULL;
    memcpy(((py_bytes*)state)->ob_sval, (void*)rng->state, RANDOM_SIZE);
    return Py_BuildValue("(((Nnn)){})", state,  prand, rb);
}
RAND_METH(pstate)       (RNGSELF) {

    return Py_BuildValue("NN",
        py_int_from_size_t(rng_PRAND_UVAL()),
        py_int_from_byte(C_BYTE(rng_PRAND_RBIT()), 'u')
        
        );
}
RAND_METH(seed)         (RNGSELF) {
   py_alias_t r = {PyBytes_FromStringAndSize(NULL, SEED_SIZE)};
   memcpy(r.bytes->ob_sval, rng->seed, SEED_SIZE);
   return r.ob;
}
RAND_METH(state)        (RNGSELF) {
    py_alias_t state = {.ob=PyTuple_New(STATE_SIZE)};
    for(int i=0; i<STATE_SIZE; ++i)
        state.tuple->ob_item[i] = PyLong_FromUnsignedLongLong(rng->state[i]);
    return state.ob;
}
FUNC(Py) statf_unpackers(randiter *ri, randiter_args_t *t, Py args) {
    #define call_impl(func) \
        return func##_iter_unpack((void*)t, args);
    RV_FLAG const flag = t->statf.impl;
    switch(flag){
        case RV_BETA:     call_impl(beta);
        case RV_EXPO:     call_impl(expo);
        case RV_GAMMA:    call_impl(gamma);
        case RV_LOGNORM:  call_impl(lognorm);
        case RV_NORM:     call_impl(normal);
        case RV_PARETO:   call_impl(pareto);
        case RV_TRI:      call_impl(triangular);
        case RV_UNI:      call_impl(uniform);
        case RV_VONMISES: call_impl(vonmises);
        case RV_WEIBULL:  call_impl(weibull);
        default:
            return PySystemError("invalid RV_FLAG");
    }
    #undef call_impl
}

DATA(py_type, NumberSet_Type) =  {
    .tp_name      = "xrand.NumberSet", 
    .tp_basicsize = sizeof(setobject),
    .tp_getattro  = PyObject_GenericGetAttr, 
    .tp_flags     = Py_TPFLAGS_DEFAULT,
    .tp_dealloc   = (destructor)setobject_dealloc,
    .tp_iter      = (getiterfunc)setobject_iter,
    .tp_repr      = (reprfunc)setobject_repr,
    .tp_new       = (newfunc)setobject_new,
};
DATA(py_type, setiter_type) =  {
    .tp_name      = "xrand.numberset_iterator", 
    .tp_basicsize = sizeof(setiter),
    .tp_getattro  = PyObject_GenericGetAttr, 
    .tp_flags     = Py_TPFLAGS_DEFAULT,
    .tp_dealloc   = (destructor)setiter_dealloc,
    .tp_iternext  = (iternextfunc)setiter_next,
    .tp_iter      = (getiterfunc)setiter_iter,
};
DATA(py_type, RandIter_Type) =  {
    .tp_name      = QUALNAME(RANDITER_NAME),
    .tp_basicsize = sizeof(randiter),
    .tp_getattro  = PyObject_GenericGetAttr,
    .tp_flags     = Py_TPFLAGS_DEFAULT,
    .tp_dealloc   = (destructor)randiter_dealloc,
    .tp_iternext  = (iternextfunc)randiter_next,
    .tp_iter      = (getiterfunc)randiter_iter,
    .tp_repr      = (reprfunc)randiter_repr,
};
DATA(py_type) PopEntry_Type = {
    .tp_name        = QUALNAME(POPENTRY_NAME),
    .tp_basicsize   = sizeof(popentry),
    .tp_new         = popentry_new,
    .tp_getattro    = PyObject_GenericGetAttr,
    .tp_flags       = Py_TPFLAGS_DEFAULT,
    .tp_dealloc     = (destructor)popentry_dealloc,
    .tp_repr        = popentry_repr,
    .tp_richcompare = popentry_richcmp,
    .tp_hash        = popentry_hash,
};
DATA(py_type) Population_Type = {
    .tp_name        = QUALNAME(POPULATION_NAME),
    .tp_basicsize   = sizeof(Population),
    .tp_new         = population_new,
    .tp_getattro    = PyObject_GenericGetAttr,
    .tp_flags       = Py_TPFLAGS_DEFAULT,
    .tp_dealloc     = (destructor)pop_dealloc,
    .tp_repr        = (reprfunc)pop_repr,
    .tp_richcompare = pop_richcmp,
    .tp_hash        = NULL,

};
DATA(py_type) Choices_Type =  {
    .tp_name      = QUALNAME(CHOICES_NAME),
    .tp_basicsize = sizeof(ChoicesObject),
    .tp_getattro  = PyObject_GenericGetAttr,
    .tp_flags     = Py_TPFLAGS_DEFAULT,
    .tp_dealloc   = (destructor)choices_dealloc,
    .tp_repr      = (reprfunc)choices_repr,
};
DATA(py_type) Dice_Type =  {
    .tp_name      = QUALNAME(DICE_NAME),
    .tp_basicsize = sizeof(DiceObject),
    .tp_getattro  = PyObject_GenericGetAttr,
    .tp_flags     = Py_TPFLAGS_DEFAULT,
    .tp_dealloc   = (destructor)dice_dealloc,
    .tp_repr      = dice_repr,
};
DATA(py_type) Random_Type  =  {
    .tp_name      = QUALNAME(RANDOM_NAME),
    .tp_basicsize = sizeof(RandomObject),
    .tp_getattro  = PyObject_GenericGetAttr,
    .tp_flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new       = (newfunc)random_new,
    .tp_dealloc   = PyObject_Del, 
};

#if defined(RAND_DEBUG) && !defined(RAND_TEST_FUNCS)
#include "tests.h"
#endif
DATA(RandomObject*)RAND = NULL;
DATA(Py) RAND_METHODS = NULL;
#include "perf.h"
#ifndef INCLUDE_PROFILER
#error no profiler 
#endif  
FUNC(Py) reload_rng(Py m, Py self, Py meths) {
    
    RandomObject *rng;
    
    rng = NULL;
    
    return PySystemError("not implemented");
}
FUNC(Py) load_rng_impl(Py m) {

    DATA(py_type)   *tp = &Random_Type;
    PyMethodDef *ml;
    Py meth;
    
    Py self = NULL;
    Py meths = NULL;
    Py r = NULL;

    if(!tp)
        return PySystemError("Random_Type deallocated");
    
    ml = tp->tp_methods; 
    if(RAND != NULL || RAND_METHODS != NULL)
        return reload_rng(m, self, meths);

    meths = __dict__();
    if(!meths)
        return NULL;

    RAND = random_new(tp, EMPTY_TUPLE, NULL);
    if(!RAND)
        goto f;
    self = (Py)RAND;

    for(; ml->ml_name != NULL; ++ml) {    
        if(ml->ml_name[0] == '_')
            continue;
        meth = PyCFunction_New(ml, self);
        if(!meth)
            goto f;
        if(PyDict_SetItemString(meths, ml->ml_name, meth))
            goto f;
        Py_DECREF(meth);
    }
    return meths;
    f:
    Py_XDECREF(meths);
    Py_XDECREF(RAND);
    return NULL;
}
FUNC(Py) load_rng(Py m) {
    Py meths, ns, r=NULL;
    meths = load_rng_impl(m);
    if(!meths)
        return NULL;
    ns =  PyModule_GetDict(m);
    if(!ns)
        return NULL;
    int v = PyDict_SetItemString(ns, "rng", (Py)RAND);
    if(_PyDict_MergeEx(ns, meths, 2)|| v) {
        Py_XDECREF(meths);
        Py_XDECREF(RAND);
        return NULL;
    }
    RAND_METHODS = meths;
    
    prn;
    
}
DATA(const char) call_perf_doc[] = (
    "perf(callable[, args, kws]) \n"
    "Number of times callable(*args, **kws) can be called per second"
);

DATA(PyMethodDef) module_methods[] = {
#   ifdef RAND_TEST_FUNCS
#error no tests
    RAND_TEST_FUNCS     
    {"py_stdev", (PyCFunction)py_stdev, METH_O, NULL},
#   endif  
    #ifdef DEBUG_PARSE_ARGS
    DEBUG_METHOD(rand64_t),
    DEBUG_METHOD(o),
    DEBUG_METHOD(d),
    DEBUG_METHOD(L),
    DEBUG_METHOD(K),
    DEBUG_METHOD(n),
    DEBUG_METHOD(N),
    DEBUG_METHOD(T),
    #endif

    #if INCLUDE_PROFILER
    {"perf", (PyCFunction)call_perf, METH_VARARGS, call_perf_doc},
    #endif
    #if INCLUDE_SETSTATE
    {"set_rng_state", (PyCFunction)random_set_state, METH_VARARGS, NULL},
    #endif
     {NULL},
};
DATA(PyModuleDef) module_ob = {
    PyModuleDef_HEAD_INIT,
    .m_name = "xrand",
    .m_doc = module_doc,
    .m_size = -1,
    .m_methods = NULL,
    .m_slots = NULL,
    .m_traverse = NULL,
    .m_clear = NULL,
    .m_free = NULL
};
TYPE_INIT(setobject) (Py m) {
    static PySequenceMethods set_as_sequence = {
        .sq_length   = (lenfunc)setobject_len,
        .sq_contains = (objobjproc)setobject_contains,
    };
    NumberSet_Type.tp_as_sequence = &set_as_sequence;
    if(setobject_init_meths(&NumberSet_Type))
        return 0;
    if(INCLUDE_NUMBERSET)
        PyModule_AddObject(m, "NumberSet", Py_NEWREF(&NumberSet_Type));
    return 1;
}
TYPE_INIT(Population) (Py m) {
    DATA(PySequenceMethods) as_sq = {
        .sq_contains = (objobjproc)pop_contains,
        .sq_length = (lenfunc)pop_length,
    };
    DATA(PyMappingMethods) as_mp = {
        .mp_subscript = (binaryfunc  )pop_get_item
    };
    if(PyType_Ready(&Population_Type) < 0)
        return FALSE;
    Population_Type.tp_as_sequence = &as_sq;
    Population_Type.tp_as_mapping = &as_mp;
    PyModule_AddObject(m, "Population", (PyObject*)&Population_Type);
    return TRUE;
}
TYPE_INIT(setiter) (Py m) {
    return PyType_Ready(&setiter_type) >= 0;
}
TYPE_INIT(randiter) (Py m) {
    return !randiter_init_meths(&RandIter_Type);
}
TYPE_INIT(PopEntry) (Py m){
    if(PyType_Ready(&PopEntry_Type) < 0)
        return FALSE;
    PyModule_AddObject(m, "PopEntry", (PyObject*)&PopEntry_Type);
    return TRUE;
}
TYPE_INIT(dice) (Py module) {
    return !dice_init_meths(&Dice_Type);
}

TYPE_INIT(Random) (Py m) {
    if(Random_init_meths(&Random_Type))
        return 0;
    if(PyModule_AddObject(m, "Random", (PyObject*)&Random_Type) < 0)
        return 0;
    return TRUE;
}
TYPE_INIT(Choices)(Py module) {
    if(choices_init_meths(&Choices_Type))
        return 0;
    return 1;
}

Py PyInit_xrand(void){
    
    Py self;

    if(!(self=PyModule_Create(&module_ob)))
        goto fail; 
    if(str_format(SEED_BITS_STR, 4, "%d", (int) SEED_BITS)<0)
        return PySystemError("sprintf");
    dict_richcomp = PyDict_Type.tp_richcompare;
    SMALL_INTS = &small_ints[TINY_INT_MAX];
    LATIN1 = small_strs;
    for(long i=TINY_INT_MIN; i<(TINY_INT_MAX+1); i++)
        SMALL_INTS[i] = PyLong_FromLong(i);
    if(((py_int*)SMALL_INTS[1])->ob_digit[0] != 1)
        return PySystemError("invalid alignment of interned tiny ints");
    for(int i=0; i<CACHED_CHARS; ++i)
        if(!(small_strs[i] = PyUnicode_FromOrdinal(i)))
            goto fail;
    if(!(EMPTY_TUPLE = __tuple__(0)))
        goto fail;
    if(!(UNICODE_EMPTY = PyUnicode_FromString("")))
        goto fail;
    if(!(UNICODE_HASH=(&PyUnicode_Type)->tp_hash))
        goto fail;
    #define _make_init_(f) f##_init_type
    #define _call_init_(f) _make_init_(f)(self)
    #define CALL_TYPE_INIT(tp, add, f) \
        if(!((add? Py_INCREF(&tp): 1), _call_init_(f))) goto fail 
    CALL_TYPE_INIT(Random_Type,     1, Random);
    CALL_TYPE_INIT(PopEntry_Type,   1, PopEntry);
    CALL_TYPE_INIT(Population_Type, 1, Population); 
    CALL_TYPE_INIT(Choices_Type,    0, Choices);
    CALL_TYPE_INIT(RandIter_Type,   0, randiter);   
    CALL_TYPE_INIT(Dice_Type,       0, dice);
    CALL_TYPE_INIT(NumberSet_Type,  0, setobject);
    CALL_TYPE_INIT(setiter_type,    0, setiter);
    int g = 1;
    if(((void*)random_next) == ((void*)xoroshiro128))
        g = PyModule_AddObject(self, "_generator", __format__("xoroshiro"));
    else if(((void*)random_next) == ((void*)xorshift128))
        g = PyModule_AddObject(self, "_generator", __format__("xorshift"));
    else
        return PySystemError("not possible");
    if(g != 0)
        goto fail;
    PyModule_AddFunctions(self, module_methods);
    INIT_SEED.i = system_clock();
    Py fin = load_rng(self);
    if(!fin)
        goto fail;
    Py_DECREF(fin);
    return self;
  fail:
    Py_XDECREF(self);
    return NULL;
}
