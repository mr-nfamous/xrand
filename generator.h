#define SAMPLEARGS_BUF (\
    sizeof(wsample_args_t) > sizeof(sample_args_t)?\
    sizeof(wsample_args_t) : sizeof(wchoice_args_t))

#define CHOICEARGS_BUF (\
    sizeof(wchoice_args_t) > sizeof(choice_args_t)?\
    sizeof(wchoice_args_t) : sizeof(choice_args_t))
#define SEED_BITS (8*SEED_BYTES)
#define SEED_BYTES (msizeof(RandomObject, seed))
#define RANDOM_SIZE msizeof(RandomObject, seed) + msizeof(RandomObject, state)
#define STATE_SIZE 2
#define SEED_SIZE offsetof(RandomObject, seed)-offsetof(RandomObject,state)
#ifndef RNG_OVERRIDE
#   ifdef RAND32
#       define XS_GENERATOR
#       define random_next xorshift128
#   else 
#       define XSR_GENERATOR
#       define random_next xoroshiro128
#   endif
#else
#   define random_next RNG_OVERRIDE
#endif

/*
The recommended constants for xoroshiro128+ were changed in 2018.
Keeping the old ones here for posterity.
*/
#ifdef USE_2016_PARAMS
#define XSR128_a 55 
#define XSR128_b 14 
#define XSR128_c 36
#define XS128_a 23 
#define XS128_b 18 
#define XS128_c 5
#define XS_JUMP 0x8a5cd789635d2dff, 0x121fd2155c472f96
#define XSR_JUMP 0xbeac0467eba5facb, 0xd86b048b86aa9922
#else 
#define XSR128_a 24 
#define XSR128_b 16 
#define XSR128_c 37
#define XS128_a 23 
#define XS128_b 18 
#define XS128_c 5
#define XS_JUMP 0x8a5cd789635d2dff, 0x121fd2155c472f96
#define XSR_JUMP 0xdf900294d8f554a5, 0x170865df4b3201fc
#endif
#if defined(XS_GENERATOR)
#   define JUMP_CONSTS XS_JUMP 
#elif defined(XSR_GENERATOR)
#   define JUMP_CONSTS XSR_JUMP
#else 
#   error("RNG jump constants were not defined.")
#endif
#ifdef RAND32
#define RBIT 32
#define PBIT 32 
#define PSHF 32
#else 
#define PSHF 0
#define RBIT 60
#define PBIT 64
#endif
#define FLOATIFY_NO_ZERO(n) (0x3ff0000000000001| (((uint64_t)(n))>>12))
#define FLOATIFY(n) (0x3ff0000000000000 | (((uint64_t)(n))>>12))
#define AKA_RNG rng

#define _rng_(o)        ((RandomObject*)(o))
#define _rng_state_(o)  _rng_(o)->state
#define _rng_seed_(o)   _rng_(o)->seed
#define _rng_rb_(o)     _rng_(o)->rb
#define _rng_pcache_(o) _rng_(o)->pcache
#define _rng_prand_(o)  _rng_pcache_(o).prand
#define _rng_uval_(o)   _rng_prand_(o).uval
#define _rng_bval_(o)   _rng_prand_(o).bval 

#define CRNG(...)       ((RandomObject*)__VA_ARGS__)
#define Random_STATE(o)      _rng_state_(o)
#define Random_SEED(o)       _rng_seed_(o)
#define Random_PRAND_UVAL(o) _rng_uval_(o)
#define Random_PRAND_BVAL(o) _rng_bval_(o)
#define Random_PRAND_RBIT(o) _rng_rb_(o)
#define Random_PRAND_MSB(o)  _rng_bval_(o).msb
#define rng_STATE(...)        (CRNG(AKA_RNG)->state)
#define rng_SEED(...)         (CRNG(AKA_RNG)->seed)
#define rng_PCACHE(...)       (CRNG(AKA_RNG)->pcache)
#define rng_PRAND(...)        (CRNG(AKA_RNG)->pcache.prand)
#define rng_PRAND_UVAL(...)   (CRNG(AKA_RNG)->pcache.prand.uval)
#define rng_PRAND_BVAL(...)   (CRNG(AKA_RNG)->pcache.prand.bval)
#define rng_PRAND_MSB(...)    (CRNG(AKA_RNG)->pcache.prand.bval.msb)
#define rng_PRAND_RBIT(...)   (CRNG(AKA_RNG)->rb)

#define rand_PNEXT(r, n, v) (\
    ((v =   ((rng_PRAND_RBIT() -= (n)),\
            (rng_PRAND_UVAL() >> (PBIT - (n))))),\
    (rng_PRAND_UVAL() <<= (n)),\
    (v))
#define random_NEXT() (random_next(rng) >> 4)
#define rand_BITS(b)          (random_next(CRNG(AKA_RNG)) >> (64-b))
#define rand_DOUBLE(...)      (random_double(CRNG(AKA_RNG)))
#define rand_SAFE_DOUBLE(...) (random_safe_double(CRNG(AKA_RNG)))
#define prand_renew(rng) \
    ((rng_PRAND_RBIT()=RBIT), (rng_PRAND_UVAL()=(random_next(rng)>>PSHF)    ))
#define check_prand(rng, nb) ((uint8_t)((rng)->rb > (nb)))
#define IMPL_ARGS(name, func) arg_struct(func) name
#define ARGS_T(name) name##_args_t
#define arg_struct(name) name##_args_t
#define RAND_ARGS(name) name##_args_t
#define RAND_ARGT(name) typedef struct name##_args_t
#define args_t(name) name##_args_t
typedef union prand_state_t {
    size_t uval;
    struct {
        size_t pad: PBIT-1;
        size_t msb: 1;
    } bval;
} prand_state_t;
typedef struct  RandomObject{
    PyObject_HEAD
    uint64_t state[STATE_SIZE];
    rand64_t seed[STATE_SIZE];
    
    struct {
        prand_state_t prand;
    } pcache;
    
    uint8_t rb;
    struct {
        uint8_t     dice : 1;
        uint8_t       x86: 1;
        uint8_t generator: 3;
        uint8_t           :3;
    } flags;
} RandomObject;
typedef Py (*roller_func)(void*);
typedef Py (*roll_impl) (RandomObject*, void*);
typedef void      (*args_destructor) (DiceObject*);

RAND_ARGT(statf) {
    double alpha, beta, lambda, kappa, mu, sigma;
} statf_args_t;
RAND_ARGT(wchoice) {

    size_t          n; // number of loops each invocation
    rand64_t      sum; // sum of the weights    
    size_t         hi; // for the hell of it let's pointlessly cache Py_SIZE
    Py            pop; // actual keys
    void         *mem; // weights
    uint8_t bisect_as; // 'i'/'d' for size_t/double bisection, respectively
} wchoice_args_t;
RAND_ARGT(choice) {
    size_t n;
    PySequence sq;
} choice_args_t;
RAND_ARGT(sample) {
    sizes_t            k;
    uint8_t            b;    
    rset        *indices;
    PySequence         s;
    PySequence       *sq;
} args_t(sample);
RAND_ARGT(wsample) {
    sizes_t            k;
    uint8_t            b;    
    rset        *indices;
    wchoice_args_t     w;
} args_t(wsample);
typedef struct wsample_args {
    sizes_t k;
    uint8_t b;
    rset *indices;
    PySequence *sq;
    wchoice_args_t *wc;
} wsample_args;
typedef struct sample_args {
    sizes_t k;
    uint8_t b;
    rset *indices;
    PySequence *sq;
    wchoice_args_t *wc;
    union {
        PySequence sq;
        wchoice_args_t wc;
    } t;
} sample_args;
typedef union sample_args_cast_t {
    char *buffer;
    void *any;
    sample_args_t *rs;
    wsample_args_t *ws;
} sample_args_cast_t;
#define WCHOICE_ARGS wc
#define RCHOICE_ARGS rc
#define WSAMPLE_ARGS rs
#define RSAMPLE_ARGS ws
#define ROLLER_ARGFIELD rawr
typedef union choice_arg_cast_t {
    char *buffer;
    void *any;
    wchoice_args_t *WCHOICE_ARGS;
    choice_args_t *RCHOICE_ARGS;
    sample_args *WSAMPLE_ARGS;
    wsample_args *RSAMPLE_ARGS;
} choice_arg_cast_t;
RAND_ARGT(randint) {
    rand64_t base;
    rand64_t width;
    uint8_t sh;
} randint_args_t;
RAND_ARGT(brandint) {
    Py big_base;
    Py big_width;
    uint8_t sh;
    digit *mem;
} brandint_args_t;
typedef union dice_arg_cast_t {
    #define RANDINT_ARGS_BUF (\
        sizeof(randint_args_t) > sizeof(brandint_args_t)?\
        sizeof(randint_args_t) : sizeof(brandint_args_t))  
    char *buffer;
    void *any;
    randint_args_t *fast;
    brandint_args_t *slow;
} dice_arg_cast_t;
typedef void*(*co_next)   (RandomObject*, void*);
typedef int  (*co_next_n) (RandomObject*, void*, Py *, size_t);
typedef void (*co_destr)  (void*);
typedef union roller_args_t {
    void *any;
    char *buffer;
} roller_args_t;
typedef struct RandRoller {
#define RandRoller_HEAD  \
    PyObject_HEAD  /*
                 ^^ have to put space after head to not make compiler explode :/*/\
    RandomObject *rng;   \
    co_next      next;   \
    co_next_n  next_n;   \
    co_destr    destr;
#define _Ro_base_(o) ((RandRoller*)(o))

#define _Ro_rng_(o)   _c_roller_(o)->rng
#define _Ro_next_(o)  _c_roller_(o)->next_n
#define _Ro_destr_(o) _c_roller_(o)->destr
#define _Ro_args_(o)  _c_roller_(o)->args.any
#define _Ro_args_as_(o,tp) ((tp)(_c_roller_(o)->args.any))     
    
    RandRoller_HEAD
    roller_args_t args;    
} RandRoller;
typedef struct ChoicesObject {
#if 1
#define _Co_base_(o) ((ChoicesObject*)(o))
#define _Co_args_(o) (_Co_base_(o)->ROLLER_ARGFIELD)
#define _Co_cargs_(o, tp) ((tp*)(o)->ROLLER_ARGFIELD)
#define _Co_args_addr_(o) _Co_args_(o, any)
#define _Co_wc_args_(o) _Co_cargs_(o, wchoice_args_t)
#define _Co_wc_tp_(o) _Co_wc_args_(o)->bisect_as
#define _Co_rc_args_(o) _Co_cargs_(o, choice_args_t)
#define _Co_ws_args_(o) _Co_cargs_(o, wsample_args)
#define _Co_rs_args_(o) _Co_cargs_(o, sample_args)
#define _Co_next_(o) (_Co_base_(o)->next_n)
#define _Co_wsum_(o) _Co_wc_args_(o)->sum
#define _Co_rsum_(o)  pyseq_SIZE(_Co_rc_args_(o))
#define _Co_call_next__(o, a, r, n)) \
    (_Co_next_(o)(_Ro_rng_(o), a, r, ((size_t)n)))

#define Choices_WC_TYPE(o) _Co_wc_tp_(o)
#define Choices_WC_ARGS(o) _Co_wc_args_(o)
#define Choices_RC_ARGS(o) _Co_rc_args_(o)
#define Choices_NEXT(o)    _Co_next_(o)
#define Choices_WSUM_D(o)  _Co_wsum_(o).d
#define Choices_WSUM_I(o)  _Co_wsum_(o).N
// doesn't work #define Choices_RSUM(o)    _Co_rsum_(o)
#define Choices_NEXT_RCHOICE(o, r, n) _Co_call_next_(o, _Co_rc_args_, r, n)
#define Choices_NEXT_WCHOICE(o, r, n) _Co_call_next_(o, _Co_wc_args_, r, n)
#define Choices_NEXT_RSAMPLE(o, r, n) _Co_call_next_(o, _Co_rs_args_, r, n)
#define Choices_NEXT_WSAMPLE(o, r, n) _Co_call_next_(o, _Co_ws_args_, r, n)
#define WCHOICE_CHECK(o) (Choices_NEXT(o) == rand_wchoice)
#endif
    RandRoller_HEAD
    choice_arg_cast_t args;
    void *ROLLER_ARGFIELD;
} ChoicesObject;
struct DiceObject {
#define _Do_base_(o) ((DiceObject*)(o))  
    RandRoller_HEAD
    dice_arg_cast_t   args;
    int generator;
    void             *ROLLER_ARGFIELD;
    
} ;
FAST_FUNC(Object)   parse_next_n    (RandRoller    *ro, Py arg, sizes_t *n) {
    if(!PyLong_Check(arg))
        return PyTypeError("n must be an integer");
    if(!py_int_as_sizes_t((py_int*)arg, n, 'n'))
        return NULL;
    if(n->i < 0)
        return PyValueError("n must be positive integer");
    return PyList_New(n->i);
}
VOID_FUNC(schoice_destr2) (choice_arg_cast_t *args) {
    PySeq_CLEAR(&args->rc->sq); 
}
VOID_FUNC(wchoice_destr2) (choice_arg_cast_t *args) {
    Py_XDECREF(args->wc->pop);
    PyObject_Free(args->wc->mem);
}
IMPL_FUNC(int)    parse_dice_args(dice_arg_cast_t *r, Py args) {
    
#   define fw (r->fast->width)
#   define fb (r->fast->base)
#   define fsh (r->fast->sh)

#   define sw (r->slow->big_width)
#   define sb (r->slow->big_base)
#   define ssh (r->slow->sh)
#   define smem (r->slow->mem)     
    
    sizes_t lo_size, hi_size;
    int result=9;

    DEFINE_VARARGS(randint, 1, 'K', 'K') = {"a", "b"};
    VARARG(Py, lo, NULL);
    VARARG(Py, hi, NULL);
    PARSE_VARARGS2(-1);

    if(hi==NULL) {
        if(Py_SIZE(lo) < 1){
            if(Py_SIZE(lo) == 0) {
                fw.i = 0, fb.i = 0;
                result = 'z';
                goto F;
            }
            if(Py_SIZE(lo) < (-DPR)) {
                hi = Py_NEWREF(SMALL_INTS[0]);
                goto S;
            }
            py_int_parse((py_int*)lo, &fb, 'i', 0);
            fw.i = 0 -(fb.i);
        }
        else {
            if(Py_SIZE(lo) > DPR) {
                hi = lo;
                lo = Py_NEWREF(SMALL_INTS[0]);
                goto S;
            }
            py_int_parse((py_int*)lo, &fw, 'i', 0);
            fb.i = 0;
        }
        goto F;
    }
    if(lo==NULL || hi==NULL)
        return PySystemError("yhhhhhh"), -1;
    py_int_sort((py_int**)&lo,  (py_int**)&hi);
    lo_size.i = Py_SIZE(lo)<0? -Py_SIZE(lo):Py_SIZE(lo);
    hi_size.i = Py_SIZE(hi)<0? -Py_SIZE(hi):Py_SIZE(hi);
    if(hi_size.u > DPR || lo_size.u > DPR)
        goto S;
    result = 'f';
    py_int_parse((py_int*)lo, &fb, 'i', 0);
    py_int_parse((py_int*)hi, &fw, 'i', 0);
    fw.i -= fb.i;
    Py_DECREF(hi);
  F:Py_DECREF(lo);
    fsh = 64 - bit_length_64(fw);
    return 'f';
  S:
    result = 's';
    if(!(sw = py_int_sub(hi, lo)))
        goto X;
    
    sb = lo;
    ssh  = PyLong_SHIFT - digit_bit_length(OB_DIGITS(sw)[Py_SIZE(sw)-1]);
    
    if((smem=PyMem_Malloc(sizeof(digit) * Py_SIZE(sw))))
        goto E;      
  X:result = -1;
    Py_DECREF(lo);
  E:Py_DECREF(hi);
    return result;
#   undef fw
#   undef fb
#   undef fs

#   undef sw
#   undef sb
#   undef ssh
#   undef smem
}
IMPL_FUNC(Object) rand_get_wchoice_args(py_dict *mp, wchoice_args_t *a) {
    // success when it returns non-null dict values
    
    Py val, values;
    a->hi        = (size_t)PyDict_GET_SIZE(mp);
    
    if(((ssize_t)a->hi) < 1)
        return (a->pop = PyValueError(empty_pop));
    a->pop = PyDict_Keys((Py)mp);
    if(!a->pop) {
        if(!PyErr_Occurred())
            PySystemError("PyDict_Keys() return null w/o an error set");
        goto fail;
    }
    
    values = PyDict_Values((Py)mp);
    if(!values) {
        if(!PyErr_Occurred())
            PySystemError("PyDict_Values() return null w/o an error set");
        goto fail;
    }
    val = PyDict_GetItem((Py)mp, LIST_ITEMS(a->pop)[0]);
    if(!val) {
        if(!PyErr_Occurred())
            PySystemError("PyDict_GetItem() return null w/o an error set");
        goto fail;
    }
    a->bisect_as = PyLong_Check(val)? 'i': (PyFloat_Check(val)? 'd': 'x');
    
    switch(a->bisect_as) {
        case 'i':
            a->mem = PyObject_MALLOC(sizeof(size_t) * a->hi);
            
            if(py_list_parse_size_t(values, a->mem, &a->sum.N))
                return values;
            break;
        case 'd':
            a->mem = PyObject_MALLOC(sizeof(double) * a->hi);
            if(py_list_parse_floats(values, a->mem, &a->sum.d))
                return values;
            break;
        case 'x':
            a->mem = PyTypeError(no_homo, Py_TP_NAME(val));
            break;
        default:
            a->mem = PySystemError("invalid ch in choiceparsedict");
    }
  fail:
    if(!PyErr_Occurred())
        PySystemError("get_wchoice_args: @fail without an error set...");
    Py_XDECREF(values);
    Py_XDECREF(a->pop);
    PyObject_Free(a->mem);

    a->hi        = 0;
    a->sum.u     = 0;
    a->bisect_as = 0;
    a->n         = 0;
    return NULL;
}
IMPL_FUNC(int) wchoice_args_from_pop(Py mp,  choice_arg_cast_t *a) {
    Py pop;
    Py values;
    values = rand_get_wchoice_args((py_dict*)mp, a->wc);
    if(!values)
        return 0;
    ssize_t const size = Py_SIZE(values);
    Py_DECREF(values);
    #ifdef RAND_DEBUG
    if(size < 1) {
        wchoice_destr2(a);
        return PySystemError("shoulda raised size err already"), 0;
    }
    #endif
    pop = PyList_New(size);
    for(ssize_t i=0; i<size; ++i) {
        Py key = PopEntry_ITEM(LIST_ITEMS(a->wc->pop)[i]);
        LIST_ITEMS(pop)[i] = Py_NEWREF(key);
    }
    Py_DECREF(a->wc->pop);
    (a->wc->pop) = pop;
    return 'm';
}
IMPL_FUNC(int)    parse_choice_args(Object arg, choice_arg_cast_t *a) {
    Py t;
    if(Py_TYPE(arg) == &Population_Type)
        return wchoice_args_from_pop(((Population*)arg)->mp, a);
    if(Py_TYPE(arg) == &PyDictProxy_Type)
        arg = ((py_dict_proxy*)arg)->mp;
    if(PyDict_Check(arg)) {
        if((t=rand_get_wchoice_args((py_dict*)arg, a->wc)))
            Py_DECREF(t);
        else
            return 0;
        #ifdef RAND_DEBUG
        if(Py_SIZE(a->wc->pop)<1) {
            wchoice_destr2(a);
            return PySystemError("shoulda raised size err already"), 0;
        }
        #endif
        return 'm';
    }
    a->rc->sq = PySequence_new();      
    if(!py_sequence_parse(arg, &a->rc->sq, 1  ))
        return 0;
    if(pyseq_SSIZE(&a->rc->sq) < 1) {
        PyValueError(empty_pop);
        schoice_destr2(a);
        return 0;
    }
    return 'r';
}
IMPL_FUNC(uint64_t)  xorshift128      (void *rng) {
    /* 
    Slightly modified from the reference implementation to help out a certain
    compiler that's apparently too dumb to optimize automatically.
    */    
    uint64_t s1 = rng_STATE()[0];
	uint64_t const result =  rng_STATE()[1] + s1;
	rng_STATE()[0] =  rng_STATE()[1];
	s1 ^= s1 << XS128_a;
	rng_STATE()[1] ^= s1  ^ (s1 >> XS128_b) ^ ( rng_STATE()[1] >> XS128_c);
	return result; 
}
IMPL_FUNC(uint64_t)  xoroshiro128     (void *rng) {
    uint64_t s0, s1;
    uint64_t const r = (s0=rng_STATE()[0]) + (s1=rng_STATE()[1]);    
    s1 ^= s0;
    rng_STATE()[0] = ROTL(s0, XSR128_a) ^ s1 ^ (s1 << XSR128_b);
    rng_STATE()[1] = ROTL(s1, XSR128_c);
    return r; 
}
IMPL_FUNC(uint64_t)  xoshiro256       (void) {
    /* 
    xoshiro's only advantage over the others is that all 64 bits pass BigCrush.
    Since Python could only ever use 60 bits of the output and since the upper
    62 bits of the other generators pass BigCrush, all using this would do is 
    reduce the speed of random_next by over 50%. 
    Either MSVC is terrible at compiling this or it isn't as fast as claimed 
    in the paper describing it.
    */
    
    static uint64_t _s0=123456;
    static uint64_t _s1=7891033;
    static uint64_t _s2=34234234234;
    static uint64_t _x3=8236482683324234;
    register uint64_t *s0 = &_s0;
    register uint64_t *s1 = &_s1;
    register uint64_t *s2 = &_s2;
    register uint64_t *s3 = &_s2;
    const uint64_t r = *s0 + *s3;
    const uint64_t t = *s1 << 17;
    *s2 ^= *s0;
    *s3 ^= *s1;
    *s1 ^= *s2;
    *s0 ^= *s3;
    *s2 ^= t;

    *s3 = _rotl64(*s3, 45);
    return r;
}
IMPL_FUNC(size_t) random_partial(RandomObject *rng, uint8_t b) {
    size_t r;
    #if !defined(RAND32)
    if(b > 59) {
        uint8_t const pbr = b - 60;//partial bits required to steal
        if(!pbr)
            return random_next(rng)>>4;
            
        if(!check_prand(rng, pbr))
            prand_renew(rng);
        r = random_next(rng) >> 4;
        r <<= pbr;
        r |= rng_PRAND_UVAL() >> (PBIT - pbr);
        rng_PRAND_UVAL() <<= pbr;
        rng_PRAND_RBIT() -= pbr;
        return r;
    }
    #endif
    if(!check_prand(rng, b))
        prand_renew(rng);
    #ifdef RAND_DEBUG
    if(b>60)
        return PySystemError("> 60 bits provided to random_partial"), 0;
    #endif
    rng_PRAND_RBIT() -= b;
    return ((r = rng_PRAND_UVAL() >> (PBIT-b)), (rng_PRAND_UVAL()<<=b), r);
}
IMPL_FUNC(Py) random_copy      (void *rng, PyTypeObject *tp) {
    if(Py_TYPE(rng) != tp)
        return PyTypeError("__copy__ requires a '%s' object; got '%s'",
            tp->tp_name, Py_TP_NAME(rng));
    RandomObject *cpy;
    if(!(cpy = CRNG(tp->tp_alloc(tp, 0))))
        return PyErr_NoMemory();
    memcpy(cpy->state, rng_STATE(), RANDOM_SIZE);
    Random_PRAND_RBIT(cpy) = Random_PRAND_RBIT(rng);
    Random_PRAND_UVAL(cpy) = Random_PRAND_UVAL(rng);
    return (Py)cpy;  
}
IMPL_FUNC(int)seed_from_int    (RNGSELF, Py arg) {
    Py seed;
    int r;
    if(!(seed = py_index(arg)))
        return 0;
    r = _PyLong_AsByteArray((py_int*)seed, rng_SEED()->str, SEED_SIZE, 1, 1);
    Py_DECREF(seed);
    if(!r)
        return 1;
    PyErr_Clear();
    PyOverflowError("signed int too large to be used as a %s bit seed",
                    SEED_BITS_STR);
    return 0;
} 
IMPL_FUNC(int)random_new_seed  (RNGSELF) {
    //Make new seed using the splitmix64 generator whose state begins as
    //the time that this module was initialized
    for(int i=0; i<STATE_SIZE; ++i) { 
        #ifdef RAND_DEBUG
        if(rng_SEED()[i].u)
            return PySystemError("Random instance had non-zero seed when "
                                 "trying to generate a new one"), 0;
        #endif
        uint64_t v = (++SEED_CTR.u)* 0x9e3779b97f4a7c15 + INIT_SEED.u;
        v = (v ^ (v >> 30)) * 0xbf58476d1ce4e5b9;
        v = (v ^ (v >> 27)) * 0x94d049bb133111eb;
        rng_SEED()[i].u = v ^ (v >> 31);
    }
    return 1;
}
IMPL_FUNC(int)random_loads     (RNGSELF, py_tuple *maybe_state) {
    
    Py args;
    
    if(Py_SIZE(maybe_state) != 3)
        goto unpickle_error;
        
    args = (Py)maybe_state;
    DEFINE_VARARGS(setstate, 3, 'o', 'n', 'n');
    VARARG(Py,     state, NULL);
    VARARG(size_t, prand, 0);
    VARARG(ssize_t, rb,   -1);
    PARSE_VARARGS3(er2);
    
    if(!PyBytes_Check(state) || PyBytes_Size(state) != RANDOM_SIZE)
        goto unpickle_error;
    if(rb<0 || rb > RBIT)
        goto unpickle_error;
    rng_PRAND_RBIT() = (uint8_t)rb;
    rng_PRAND_UVAL() = (size_t)prand;
    memcpy(rng_STATE(), ((py_bytes*)state)->ob_sval, RANDOM_SIZE);
    return TRUE;
    unpickle_error:
    return PyTypeError("invalid state for reconstructing Random object"), 0;  
    er2:
    return 0;
    return PyTypeError("Random unpickler got invalid types/values"), 0;
} 
IMPL_FUNC(int)random_init_seed (RNGSELF) { 
    rand64_t *seed = ((RandomObject*)rng)->seed;
    for(int i = STATE_SIZE; i; --i, ++seed)
        if(seed->u)
            return TRUE;
    return PyValueError("Random seed must not be all-zero"), 0;
}
IMPL_FUNC(double) random_double(RNGSELF) {
    rand64_t r;
    return (r.u = FLOATIFY(random_next(rng))), r.d - 1.0;
}
IMPL_FUNC(double) random_safe_double(RNGSELF) {
    // at 7.0024 ns per double generated, one can expect to encounter
    // the 0.0 double once per 365.0000009 days.
    rand64_t r;
    return (r.u = FLOATIFY_NO_ZERO(random_next(rng))), r.d - 1.0;
}
IMPL_FUNC(int)    random_jump      (RNGSELF) {
    static const uint64_t jump[] = { JUMP_CONSTS };
    #ifdef RAND_DEBUG
    if((sizeof(jump)/sizeof(*jump))!= STATE_SIZE)
        return PySystemError("jump constant / state size mismatch"),0;
    #endif
    uint64_t s0=0, s1=0;
    for(int i=0; i<STATE_SIZE; ++i)
        for(int j=0; j<64; ++j) {
            if(jump[i] & 1ULL << j)
                s0 = s0^rng_STATE()[0], s1 = s1 ^ rng_STATE()[1];
            random_next(rng);
        }
    rng_STATE()[0] = s0;
    rng_STATE()[1] = s1;
    return 1;
}
IMPL_FUNC(void)   random_digits(RNGSELF, digit *p, size_t n) {
    rand64_t x;
#   if(defined(RAND32))
#       define FAST_DIGITS(x) (x&D0) | (x&D1)<<1 | (x&D2)<<2 | (x&D3)<<3
#   else
#       define FAST_DIGITS(x) (x&D0) | (x&D1) << 2
#   endif
    A:
    if(!n) 
        return;
    x.u = random_next(rng) >> 4;   
    if(n>=DPR) {
        #ifdef RAND32
        uint64_t av, bv, cv;
        av = x.u & 0xfffe00000000000;
        bv = av << 3;
        av = x.u & 0x0001fffc0000000;
        cv = av << 2;
        bv|= cv;
        av = x.u & 0x00000003fff8000;
        cv = av << 2;
        cv = av << 1;
        av = x.u & 0x000000000007fff;
        rand64_t const v = {.u = av | bv | cv};
        p[0] = v.p[0];
        p[1] = v.p[1];
        p[2] = v.p[2];
        p[3] = v.p[3];
        p += DPR;
        #else
        ((rand64_t*)p)->u = FAST_DIGITS(x.u);
        ++((rand64_t*)p);
        #endif
        n -= DPR;
        goto A;
    }
    goto C;
    B:
    if(!n)
        return;
    x.u >>= PyLong_SHIFT;
    ++p;
    --n;
    C:
    *p = (digit)(x.u & PyLong_MASK);
    goto B;
}
IMPL_FUNC(void)   random_bytes(RNGSELF, uint8_t *p, size_t n) {
    /*
    Don't remember if I promised that the lowest 4 bits will literally
    NEVER be used but if I did, I lied.
    
    No matter what I try, there is no situation where urandom doesn't 
    begin to beat the performance of this somehow... And with this,
    version it still quickly begins to match the speed.
    
    Figures are in MB/s generated at the Python level with the statements:
    (this is for 32 bit; at 10000 bytes per call, 64 bit is still 22% ahead)  
    
    urand = "urandom(n)"
    xrand = "rand_bytes(n)"
         
        n   xrand   urand
        5   0.048   0.027
       10   0.085   0.051
       50   0.390   0.168
     1000   1.576   0.942
     2500   1.904   1.340
     5000   2.049   1.576
     7500   2.089   1.637
    10000   2.120   1.729
    
    10 bytes per call = 8,500,000 calls per second
    50 bytes per call = 7,750,000 calls per second
    
    CRAZY.
    */
    rand64_t bits = {0};
    uint8_t const * bytes = bits.str;
  A:
    if(!n)
        return;
    bits.u = random_next(rng);
    if(n > 7) {
        memcpy(p, bytes, 8);
        n -= 8;
        p += 8;
        goto A;
    }
    else
        memcpy(p, bytes, n);   
}
IMPL_FUNC(RandomObject*) random_new(py_type *cls, Py args, Py kw) {  
    
    RandomObject *self;
    
    DEFINE_VARARGS(Random, 0, 'o') = {"arg"};
    VARARG(Py , arg, NULL);
    if(!no_kwargs("Random", kw))
        return NULL;
    PARSE_VARARGS();            
    
    self = __object__(cls);
    if(!self)
        return NULL;
    
    if(arg==NULL) {
        if(random_new_seed(self))
            goto check_seed; 
        goto fail;
    }
    if(PyTuple_CheckExact(arg)) {
        if(random_loads(self, (py_tuple*)arg))
            goto initialized;
        goto fail;
    }
    
    if(PyObject_CheckBuffer(arg)) {
        Py_buffer buffer_view;
        if (PyObject_GetBuffer(arg, &buffer_view, PyBUF_SIMPLE))
            goto fail;
        ssize_t nbytes = buffer_view.len>SEED_SIZE? SEED_SIZE: buffer_view.len;
        memcpy(self->seed, buffer_view.buf, nbytes);
        PyBuffer_Release(&buffer_view);
        goto check_seed; 
    }     
    if(PyIndex_Check(arg)) {
        if(seed_from_int(self, arg) && !PyErr_Occurred())
            goto check_seed;
        goto fail;
    }
    PyTypeError("Random seed be an int or object supporting the buffer protocol");
  fail:
    Py_XDECREF(self);
    return NULL;        
  check_seed:
    if(!random_init_seed(self))
        goto fail;
    memcpy(self->state, self->seed, msizeof(RandomObject, state));
  initialized:
    return self;
}
IMPL_FUNC(size_t) next_index (RNGSELF, sizes_t *index) {
    size_t r;
    uint8_t const b = size_t_bit_length(*index); 
    do {
        r = random_partial(rng, b);
    } while(r>=index->u);
    return r;
}
FUNC(int) check_sample_k(sample_args *a,  sizes_t *size) {
    
    if(size->i< 1)
        return PyValueError("there's no population to sample here"), 0;
    if(a->k.i > size->i)
        return PyValueError("sample size k greater than population size"), 0;
    a->b = size_t_bit_length(size->u);
    return 1;
}
FAST_FUNC(int) parse_wsample(Py pop, sample_args *a) {
    Py t = rand_get_wchoice_args((py_dict*)pop, &a->t.wc);
    if(!t)
        return 0;
    Py_DECREF(t);
    if(a->t.wc.bisect_as != 'i')
        return PyTypeError("cannot sample using float-based weighting"), 0;
    if((!check_sample_k(a, &a->t.wc.sum.c)) || (!(a->indices=new_set(&a->k)))) {
        Py_XDECREF(a->t.wc.pop);
        PyObject_Free(a->t.wc.mem);
        return 0;
    }
    a->wc = &a->t.wc;
    return 'w';
}
FAST_FUNC(int) parse_rsample(Py pop, sample_args *a) {

    if(!py_sequence_parse(pop, &a->t.sq, 1))
        return 0;
    if(!check_sample_k(a, pyseq_SIZES(&a->t.sq)))
        goto A;
    if(!(a->indices = new_set(&a->k)))
        goto A;
    a->sq = &a->t.sq;
    return 'r';
    A:
    PySeq_CLEAR(&a->t.sq);
    return 0;
}
FAST_FUNC(int) parse_sample_args(Py pop, sample_args *a) {
    if(Py_TYPE(pop) == &PyDictProxy_Type)
        pop = ((py_dict_proxy*)pop)->mp;
    if(PyDict_Check(pop))
        return parse_wsample(pop, a);
    
    return parse_rsample(pop, a);
}
FUNC(Py) choices_next   (ChoicesObject*);
