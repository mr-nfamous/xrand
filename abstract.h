#define py_float_FROM_RAND64_T(a) PyFloat_FromDouble(a.d)
#ifndef PyDict_GET_SIZE
#define PY_NOHASH PyObject_HashNotImplemented
#define ERROR() (PyThreadState_GET()->curexc_type)
#define NO_ERROR() (!ERROR())

#define PyDict_GET_SIZE(mp) (((py_dict*)mp)->ma_used)
#endif
#define Py_SIZES(o) ((sizes_t*)(&((PyVarObject*)(o))->ob_size))
#define SIMPLE_TYPE_ERROR(ob, ...) \
    PyTypeError( __VA_ARGS__ " (got type '%s')", Py_TP_NAME(ob))
#define pbv Py_BuildValue 
#define prt(x) return ((x)? (Py_NEWREF(Py_True)): (Py_NEWREF(Py_False)))
#define set_error(exc, msg, ...) PyErr_Format(PyExc_##exc, msg, __VA_ARGS__)
#define PyIndexError(msg, ...) set_error(IndexError, msg, __VA_ARGS__)
#define PyKeyError(msg, ...) set_error(KeyError, msg, __VA_ARGS__)
#define FastKeyError(k) \
    set_error(KeyError, "%R", (Object )(((FastDictKey*)k)->key))
#define PyValueError(msg, ...) set_error(ValueError, msg, __VA_ARGS__)
#define PyTypeError(msg, ...) set_error(TypeError, msg, __VA_ARGS__)
#define PySystemError(msg, ...) set_error(SystemError, msg, __VA_ARGS__)
#define PyOverflowError(msg, ...) set_error(OverflowError, msg, __VA_ARGS__)
#define PyZeroDivisionError(msg, ...) set_error(ZeroDivisionError, msg, __VA_ARGS__)
#define PyNoError(fname) PySystemError(#fname "failed without an exception being set")
#define system_clock _PyTime_GetSystemClock
#define time_as_double() _PyTime_AsSecondsDouble(system_clock())
#if !defined(_py_slot_)
#define _py_slot_(slot) tp_as_##slot
#define Py_TP_ALLOC(ob, tp, items) ((ob*)((tp)->tp_alloc((tp), (items))))
#define Py_TP_NAME(ob) (Py_TYPE((ob))->tp_name)
#define Py_TP_DICT(ob) (Py_TYPE(ob)->tp_dict)
#define Py_TP_REPR(ob) (Py_TYPE(ob)->tp_repr)
#define CALL_TP_REPR(ob) (Py_TYPE(ob)->tp_repr((ob)))
#define Py_TP_METHODS(ob) (Py_TYPE(ob)->tp_methods)
#define Py_TP_GET(ob) (Py_TYPE(ob)->tp_descr_get)
#define Py_TP_SET(ob) (Py_TYPE(ob)->tp_descr_set)
#define Py_TP_HASH(ob) (Py_TYPE(ob)->tp_hash)
#define CALL_TP_HASH(ob) (Py_TYPE(ob)->tp_hash((ob)))
#define Py_TP_ITER(ob) (Py_TYPE(ob)->tp_iter)
#define Py_TP_NEXT(ob) (Py_TYPE(ob)->tp_next)
#if !defined Py_TP_RICHCOMPARE
#   define _richcmp_(o)             ((o)->ob_type->tp_richcompare)
#   define _richcmp_call_(a, b, op) ((a)->ob_type->tp_richcompare(a,b,op))
#   define Py_TP_RICHCOMPARE(ob) (((Py)(o))->ob_type->tp_richcompare)
#   define CALL_TP_RICHCOMPARE(a, b, op) \
        richcmp_call_(_c_(Py, a), _c_(Py, b), (op))

#   define RICHCMP_EQ(a, b) _richcmp_call_(a, b, Py_EQ)
#   define RICHCMP_NE(a, b) _richcmp_call_(a, b, Py_NE)
#   define RICHCMP_LT(a, b) _richcmp_call_(a, b, Py_LT)
#   define RICHCMP_GT(a, b) _richcmp_call_(a, b, Py_GT)
#   define RICHCMP_LE(a, b) _richcmp_call_(a, b, Py_LE)
#   define RICHCMP_GE(a, b) _richcmp_call_(a, b, Py_GE)
#endif  
#if !defined(Py_TP_MAPPING)
#define Py_TP_MAPPING(ob) (Py_TYPE(ob)->tp_as_mapping)
#endif
#if !defined(Py_TP_NUMBER)
#   define Py_TP_NUMBER(ob) (Py_TYPE(ob)->tp_as_number)
#   define _nb_slot_(a, o) (o->ob_type->tp_as_number->nb_##a)
#   define _nb_call_(a, o, ...) _nb_slot_(a, o)(__VA_ARGS__)

#   define Py_NB_FLOAT(ob) _nb_slot_(float, (ob))
#   define Py_NB_INT(ob)   _nb_slot_(int,   (ob))
#   define Py_NB_INDEX(ob) _nb_slot_(index, (ob))

#   define NB_INDEX(ob)    _nb_call_(index, ((Py)(ob)), ob)
#   define NB_INT(ob)      _nb_call_(int,   ((Py)(ob)), ob)
#   define NB_FLOAT(ob)    _nb_call_(float, ((Py)(ob)), ob)
#endif
#define Py_TP_SEQUENCE(ob) (Py_TYPE(ob)->tp_as_sequence)
#   define _sq_slot_(a, o) (o->ob_type->tp_as_sequence->sq_##a)
#   define _sq_call_(a, o, ...) _sq_slot_(a, o)(__VA_ARGS__)

#   define Py_SQ_ITEM(ob) _sq_slot_(item, (ob))
#   define Py_SQ_LENGTH(ob) _sq_slot_(length, (ob))
#   define Py_SQ_ASS_ITEM(ob) _sq_slot_(ass_item, (ob))

#   define SQ_ITEM(ob, ...) _sq_call_(item, __VA_ARGS__)
#   define SQ_LENGTH(ob, ...) _sq_call_(length, __VA_ARGS__)
#endif
#define isinstance(ob, tp) (((ob)->ob_type)==(&(tp)))
#define is_neg_ssize_t(v) (((size_t)(v)) >> (SIZEOF_SIZE_T * 8 - 1))
#define OB_DIGITS(n) (((py_int*)(n))->ob_digit)
#define TUPLE_UPDATE(ob, i, ...) (TUPLE_ITEMS(ob)[i] = (__VA_ARGS__))
#define LIST_ITEMS(ob) (((py_list*)(ob))->ob_item)
#define TUPLE_ITEMS(ob) (((py_tuple*)(ob))->ob_item)
#define Py_NEWREF(o) (Py_INCREF((o)), ((Object )(o)))

#define Py_RETURN_TRUTH(t) return Py_TRUTH(t)
#define Py_WAS_FALSE ((Py_INCREF(Py_False)), ((Py)Py_False))
#define Py_WAS_TRUE ((Py_INCREF(Py_True )), ((Py)Py_True))
#define Py_TRUTH(o) ((o)? Py_WAS_TRUE: Py_WAS_FALSE)
#define Py_RETURN_TRUTH(o) return Py_TRUTH(o)

#define Py_RETURN_ZERO return Py_NEWREF(SMALL_INTS[0])
#define Py_RETURN_SMALL(n) return Py_NEWREF(SMALL_INTS[n])
#define py_float_FROM_RAND64_T(a) PyFloat_FromDouble(a.d)
#define py_int_add(x, y) \
  (PyLong_Type.tp_as_number->nb_add((Object )(x), (Object )(y)))
#define py_int_sub(x, y) \
  (PyLong_Type.tp_as_number->nb_subtract((Object )(x), (Object )(y)))
#define py_int_parse_ssize_t(o, v) py_int_parse(((py_int*)o), v, 'n', FALSE)
#define py_int_copy(n) \
    ( py_int_from_digits(((py_int*)(n))->ob_digit, Py_SIZE(n)))

#define _c_sq_(o)      ((PySequence*)(o))

#define _sq_base_(o)    (_c_sq_(o)->base)
#define _sq_range_(o)   (_c_sq_(o)->range)
#define _sq_fast_(o)    (_c_sq_(o)->fast)
#define _sq_mapping_(o) (_c_sq_(o)->mapping)
#define _sq_unicode_(o) (_c_sq_(o)->unicode)

#define _sq_size_(o)  (_sq_base_(o).size)
#define _sq_m_(o)     (Py_TYPE(_sq_sq_(o))->tp_as_sequence)
#define _sq_sq_(o)    ((Object )(_sq_base_(o).sq))
#define _sq_item_(o)  (_sq_base_(o).sq_item)
#define _sq_sizes_(o) ((sizes_t*)(&_sq_size_(o)))
#define pyseq_SQ_M_LENGTH(o) SQ_LENGTH(_sq_sq_(o))
#define pyseq_SQ_M_ITEM(o, i) (_sq_m_(o)->sq_item(_sq_sq_(o), (i)))
#define pyseq_SQ_M_ASS_ITEM(o, i, v) (_sq_m_(o)->sq_ass_item(o, (i), (v)))
#define pyseq_TYPE(o) Py_TYPE(_sq_sq_(o))
#define pyseq_NAME(o) Py_TP_NAME(_sq_sq_(o))
#define pyseq_ITER(o) Py_TP_ITER(_sq_sq_(o))
#define pyseq_SQ(o)      _sq_sq_(o)

#define pyseq_SET_SQ(sq, v)   (_sq_sq_(sq)   = ((Object )(v)))
#define pyseq_SET_SIZE(sq, v) (_sq_size_(sq) = ((ssize_t)  (v)))
#define pyseq_SET_SQ_M(sq, v)   (_sq_sq_(sq)   = ((Object )(v)))
#define pyseq_SQ_AS_LIST(o) ((py_list*)_sq_sq_(o))
#define pyseq_SQ_AS_TUPLE(o) ((py_tuple*)_sq_sq_(o))
#define pyseq_SIZES(o)   _sq_sizes_(o)
#define pyseq_SSIZE(o)   _sq_size_(o)

#define pyseq_SIZE(o)    ((size_t)(_sq_size_(o)))
#define pyseq_SQ_M(o)    _sq_m_(o)
#define pyseq_SQ_ITEM(o) _sq_item_(o)
#define pyseq_GET_ITEM(o, i, r) \
    (_sq_item_(o)(_c_sq_(o), (ssize_t)(i), ((Object *)r)))
 
//FastSequence macros
#define _sq_fast_as_(c,t,s) ((t)((c)? _sq_sq_(s): NULL))
#define _sq_fast_tuple_(s) _sq_fast_as_(PyTuple_Check(_sq_sq_(s)), py_tuple*, s)
#define _sq_fast_list(s)   _sq_fast_as_(PyList_Check( _sq_sq_(s)), py_list*,  s)
#define PySeq_AS_TUPLE(s) _sq_fast_tuple_((s))
#define PySeq_AS_LIST(s)  _sq_fast_list((s))
    
#define PySeq_AS_FAST(o) (\
    (PyTuple_Check((o)->base.sq) || PyList_Check((o)->base.sq))?\
    ((o)->base.sq): NULL)
#define FastSequence_OB_ITEM(o)  (_sq_fast_(o).ob_item)
#define FastSequence_ITEM(o, i) Py_NEWREF(FastSequence_OB_ITEM(o)[(i)])
//RangeSequence macros
#define PySeq_AS_RANGE(s) \
    (PyRange_Check(_sq_sq_(s))? _sq_range_(o): NULL)
#define RangeSeq_START(o) (_sq_range_(o).start)
#define RangeSeq_STOP(o)  (_sq_range_(o).stop)
#define RangeSeq_STEP(o)  (_sq_range_(o).step)
#define RangeSeq_ITEM(o, i) \
    py_int_from_ssize_t(RangeSeq_START(o) + RangeSeq_STEP(o) * (i))

//MappingSequence 
#define MappingSeq_KEYS(o) (_sq_mapping_(o).keys)
#define MappingSeq_VALUES(o) (_sq_mapping_(o).values)
//UnicodeSequence macros
#define PySeq_AS_UNICODE(s) (PyUnicode_Check(_sq_sq(s))? _sq_unicode_(s): NULL)
#define UnicodeSeq_MAXSIZE(o) (_sq_unicode_(o).maxsize)
#define UnicodeSeq_DATA(o)\
    (_sq_unicode_(o).data)
#define UnicodeSeq_DLEN(o) (_sq_unicode_(o).dlen)

#define PySeq_NEW(...) {-1, NULL, NULL, NULL}

#define PySeq_CLEAR(o) Py_DECREF(pyseq_SQ(o))
#define PySeq_FREE(o) (PyObject_FREE(PySeq_SQ(o)))
#ifdef RAND32
#define digit_bit_length(d) (BL16((BL16_t*)(&(d))))
#define size_t_bit_length(d) (BL32((BL32_t*)(&(d))))
#else 
#define digit_bit_length(d) (BL32((BL32_t*)(&(d))))
#define size_t_bit_length(d) (BL64((rand64_t*)(&(d))))
#endif
#define bit_length_64(d) (BL64((rand64_t*)(&(d))))
#define VOID_FUNC(f) static __inline void f
#define BOOL_FUNC(f) static int f
#define UTIL_FUNC(f) static void *f
void *__object__(PyTypeObject *tp) {
    /* up to 6% faster than a PyObject_New call */
    size_t const size = (size_t)tp->tp_basicsize;
    void *mem = PyObject_Malloc(size);
    if(!mem)
        return PyErr_NoMemory();
    memset(mem, 0, size);
    return PyObject_INIT(mem, tp);
}
void *__var_object__(PyTypeObject *tp, size_t const size) {
    
    /* does not zero out the variable length part */
    
    py_alias_t mem;
    //char *mem;
    
    size_t const head  = (size_t) tp->tp_basicsize;
    size_t const item  = (size_t) tp->tp_itemsize;
    size_t const tail  = size * item;
    size_t const total = _Py_SIZE_ROUND_UP(head+tail, SIZEOF_VOID_P);

    mem.any = PyObject_Malloc(total);
    if(!mem.any)
        return (void*)PyErr_NoMemory();
        
    memset(mem.any, 0, offsetof(PyVarObject, ob_size));
    PyObject_INIT(mem.ob, tp);
    mem.var->ob_size = (ssize_t)size;
    
    return mem.any;
}
FAST_FUNC(PySequence) PySequence_new(void) {
    PySequence const r = {NULL, -1, NULL};
    return r;
}
FAST_FUNC(void) PySequence_clear(PySequence *sq) {
    Py_XDECREF(sq->base.sq);
}
FUNC(void) GenericPySequence_clear(GenericPySequence *sq) {
    Py_XDECREF(sq->sq);
    sq->size = -1;
    sq->sq_item = NULL;
}
#define FastSequence_clear(sq) GenericPySequence_clear((GenericPySequence*)sq)
#define PySequence_GET_ITEM(sq, index, item) \
    sq->sq_item? sq->sq_item(sq, i): 
typedef union {
    uint16_t d;
    struct {
        uint16_t q1: 8;
        uint16_t q2: 8;
    } half;
} BL16_t;
typedef union {
    uint32_t d;
    struct {
        uint32_t q1: 8;
        uint32_t q2: 8;
        uint32_t q3: 8;
        uint32_t q4: 8;
    } qtr;
} BL32_t;
typedef struct _low60 {
    uint32_t w0;
    uint32_t w1;
} _low60;
FAST_FUNC(uint8_t) BL16(BL16_t *msd) {
    if(msd->d > 255)
        return BIT_LENGTHS[msd->half.q2] + 8;
    return BIT_LENGTHS[msd->d];
};
FAST_FUNC(uint8_t) BL32(BL32_t *msd) {
    if(msd->d > 65535) {
        if(msd->d > 16777215)
            return BIT_LENGTHS[msd->qtr.q4] + 24;
        return BIT_LENGTHS[msd->qtr.q3] + 16;
    }
    if(msd->d > 255)
        return BIT_LENGTHS[msd->qtr.q2] + 8;
    return BIT_LENGTHS[msd->qtr.q1];
}
FAST_FUNC(uint8_t) BL64(rand64_t *v) {
    if(v->w[1]) {
        if(v->str[7])
            return BIT_LENGTHS[v->str[7]] + 56;
        if(v->str[6])
            return BIT_LENGTHS[v->str[6]] + 48;
        if(v->str[5])
            return BIT_LENGTHS[v->str[5]] + 40;
        return BIT_LENGTHS[v->str[4]] + 32;
    }
    if(v->w[0] < 65536) {
        if(v->str[1])
            return BIT_LENGTHS[v->str[1]] + 8;
        return BIT_LENGTHS[v->str[0]];
    }
    if(v->str[3])
        return BIT_LENGTHS[v->str[3]] + 24;
    return BIT_LENGTHS[v->str[2]] + 16;
}
FUNC(uint8_t) uint32_bit_length(uint32_t *v) {
    if(*v > 65535) {
        if(*v > 16777215)
            return BIT_LENGTHS[((uint8_t*)v)[3]] + 24;
        return BIT_LENGTHS[((uint8_t*)v)[2]] + 16;
    }
    if(*v > 255)
        return BIT_LENGTHS[((uint8_t*)v)[1]] + 8;
    return BIT_LENGTHS[*v];
}
FUNC(int)       digit_cmp(digit *x, digit *y, ssize_t size) {
    --size;
    for(digit *xx=x+size, *yy=y+size; size>=0; --xx, --yy, --size) {
        if(*xx > *yy)
            return 1;
        if(*xx < *yy)
            return -1;
    }
    return 0;
}
BOOL_FUNC(cmp_gt)(digit *x, digit *y, ssize_t len) {
    return digit_cmp(x, y, len)==1;
}
FAST_FUNC(int)   cmp_ge(digit *x, digit *y, ssize_t len) {
    return digit_cmp(x, y, len)>=0;
}
FAST_FUNC(int)   cmp_eq(digit *x, digit *y, ssize_t len) {
    return digit_cmp(x, y, len)==0;
}
FAST_FUNC(int)   cmp_lt(digit *x, digit *y, ssize_t len) {
    return digit_cmp(x, y, len) < 0;
}
VOID_FUNC(py_int_sort) (py_int **a, py_int **b) {
    //minimax: swap the values of a and b if a > b
    //py_int *x=*a, *y=*b;
    py_int *c, *x=*a, *y=*b;
    if(Py_SIZE(x) < Py_SIZE(y))
        return;
    if(Py_SIZE(x)==Py_SIZE(y))
        if(cmp_lt(x->ob_digit, y->ob_digit, Py_SIZE(x)))
            return;
   c = x; *a = *b; *b = c;
}
#define OVERFLOWS_AS(__c_type__) \
    PyOverflowError("Python int cannot fit in a C '" #__c_type__ "'")
#define SET_SIZE_AND_OVERFLOWED(__c_type__, __p__, __var__, __ob_size__) \
    ((__var__ = (overflows_as_##__c_type__ (__p__, __ob_size__))) < 0)

FAST_FUNC(ssize_t)   overflows_as_int64_t(const digit *p, ssize_t size) {    
    const ssize_t sv = size<0? -size: size;
    if( sv > DPR )
        #define __any__ (((_low60*)p)->w0 | ((_low60*)p)->w0)
        if((sv>(DPR+1))||(p[DPR]>8)||((p[DPR]==8)&&((sv==size)||__any__)))
            return -1;
        #undef __any__
    return sv;
}
FAST_FUNC(ssize_t)   overflows_as_uint64_t(const digit *p, ssize_t size) {
    return size<0?-1:(size>DPR?(size>(DPR+1)?-1:(p[DPR]<16?size:-1)):size);          
}
#ifdef RAND32
#   define DP32 3
#   define overflows_as_ssize_t overflows_as_int32_t
#   define overflows_as_size_t overflows_as_uint32_t
#else
#   define DP32 2
#   define overflows_as_size_t overflows_as_uint64_t 
#   define overflows_as_ssize_t overflows_as_int64_t
#endif
FAST_FUNC(ssize_t)   overflows_as_int32_t(const digit *p, ssize_t size) {
    const ssize_t sv = size<0? -size: size;
    #define __any__ (((_low60*)p)->w0)
    if((sv>DP32) || ( // > digits in int32_t
        (sv==DP32) && (
            (p[DP32-1]>2) || // high digit > 2 means > 2**31
            // if size==sv then it's unsigned (overflows)
            ((p[DP32-1]==2) && ((sv==size)||__any__)))))
        return -1;
    #undef __any__
    return sv;
}
FAST_FUNC(ssize_t)   overflows_as_uint32_t(const digit *p, ssize_t size) {
    if(size<DP32)
        return size<0? -1: size;
    if(size>DP32 || (size==DP32 && p[DP32-1]>3))//>2 for signed
        return -1;
    return size;
}
FAST_FUNC(Object )   py_long_old(size_t size, void *ob_digit, const bint sign) {   
    if(size==1) {
        if(((digit_t*)ob_digit)->u<257)
            switch(sign) {
                case 0:
                    Py_RETURN_SMALL(*((digit*)ob_digit));
                default:
                    Py_RETURN_SMALL(-(*((sdigit*)ob_digit)));                   
            }            
    }
    size_t nbytes = size*(sizeof(digit));
    py_alias_t r = { PyObject_MALLOC(offsetof(py_int, ob_digit) +  nbytes)};
    if(!r.any)
        return NULL;
    PyObject_INIT_VAR(r.ob, &PyLong_Type, size);
    memcpy(r.int_->ob_digit, ob_digit, nbytes);
    if(sign)
        r.var->ob_size = -((ssize_t)size);
    return r.ob;
}
FAST_FUNC(Object )   py_long_new(size_t size, void *ob_digit, const bint sign) {   
    
    py_alias_t r;
    digit_t *const v = _c_(digit_t *const, ob_digit);
    if(size==1)
        if(_c_(digit_t*, ob_digit)->u < 257) {
            r.ob = sign? SMALL_INTS[-v->i]: SMALL_INTS[v->u];
            return (++r.ob->ob_refcnt, r.ob);
        }
    r.any = __var_object__(&PyLong_Type, size);
    if(!r.any)
        return NULL;
    #ifdef RAND32
    memcpy(r.int_->ob_digit, ob_digit, size << 1);
    #else 
    memcpy(r.int_->ob_digit, ob_digit, size << 2);
    #endif
    if(sign)
        r.var->ob_size = -r.var->ob_size;
    return r.ob;
}
FAST_FUNC(Object )   py_int_from_a_digit(digit *ob_digit) {   
    py_alias_t r;
    if(*ob_digit < 257)
        Py_RETURN_SMALL(*ob_digit);
    if(!(r.ob = PyObject_MALLOC(sizeof(py_int))))
        return NULL;
    PyObject_INIT_VAR(r.ob, &PyLong_Type, 1);
    r.int_->ob_digit[0] = *ob_digit;
    return r.ob;
}
BOOL_FUNC(py_int_parse) (py_int *arg, rand64_t *v, int fmt, bint overflow) {        
 /* The format codes for converting from python ints to C types are:
    I - int64_t, no check for overflow
    U - uint64_t, no check for overflow 
        Note that py_int_from_rand64_t assumes its argument is signed,
        so unsigned values greater than 2**63-1 will underflow beginning 
        at -2**63 as what would happen if casting a uint64_t to int64_t
    i - int64_t, checks for overflow
    u - uint64_t, checks for overflow
    N - size_t, checks for overflow 
    n - ssize_t, checks for overflow
    
    To increase performance, the format code is only checked if
    overflow isn't allowed. As a rule, uppercase codes are free to 
    overflow. However, all internal users of this function will 
    ensure that N/n do NOT overflow.
    
    return TRUE on success, FALSE otherwise.
*/
    ssize_t auto  size = Py_SIZE(arg);
    int     const sign = size < 0;    
    digit   const   *p = arg->ob_digit;
    if(!size)
        return (v->i = 0), TRUE;
    if(overflow) {
        size = sign? -size: size;
        goto A;
    }
    switch(fmt) {
#   ifndef RAND64_T_OVERFLOW
        #define RAND64_T_OVERFLOW(ch, __c_type__) \
            case ch:\
                if(SET_SIZE_AND_OVERFLOWED(__c_type__, p, size, Py_SIZE(arg)))\
                    return OVERFLOWS_AS(__c_type__), FALSE;\
                break;
        RAND64_T_OVERFLOW('i', int64_t)
        RAND64_T_OVERFLOW('u', uint64_t)
        RAND64_T_OVERFLOW('n', ssize_t)
        RAND64_T_OVERFLOW('N', size_t)
#       undef RAND64_T_OVERFLOW
#   else 
        RAND64_T_OVERFLOW was redifined
#   endif        
        default:
            return 0;
    }
  A:
    for(v->u = p[--size]; size; ) {
        v->u <<= PyLong_SHIFT;
        v->u |= p[--size];
    }
    if(sign)
        v->i = -v->i;
    return TRUE;
}
BOOL_FUNC(py_float_as_finite) (py_float *arg, double *d) {
    if(!Py_IS_FINITE(arg->ob_fval))
        return 0;
    return (*d=arg->ob_fval), 1;
}
FAST_FUNC(uint64_t) py_int_as_uint64_t(Object v) {        
    //uint64_t with zero overflow checks
    // One of the first thing I ever made in C. It works well enough,
    // but it probably isn't optimal. I just couldn't convince myself
    // to let it go.
    #define OB_DIGIT(v, i) (((py_int*)v)->ob_digit[i])
    ssize_t size;   
    rand64_t n = {.u=0};
    twodigits t;
    size = Py_SIZE(v);
    if(size < 2)
        switch(size) {
            case 1: return OB_DIGIT(v, 0);
            case 0: return 0;
            case -1:n.i = -((sdigit)OB_DIGIT(v, 0)); return n.u;
            default: size = -size;
        }    
    if(size>3)
        n.w[1]  = OB_DIGIT(v, 3) << PyLong_SHIFT;
    if(size>2) {
        n.w[1] |= OB_DIGIT(v, 2);
        n.u >>=2;
        if(size>DPR)
            n.str[7] |= ((uint8_t)OB_DIGIT(v, DPR)<<4);
    }
    if(size>1) {
        t = OB_DIGIT(v, 1);
        t <<= PyLong_SHIFT;
        t |= OB_DIGIT(v, 0);
        n.w[0] |= t;              
    }
    else 
        n.u = OB_DIGIT(v, 0);
    if(Py_SIZE(v)<0)
        n.i = -n.i;
    return n.u;
}
BOOL_FUNC(py_number_as_double) (Object arg, double *d) {
    Object f;
    if(PyFloat_Check(arg))
        return (*d=PyFloat_AS_DOUBLE(arg)), 1;
    if(!(Py_TP_NUMBER(arg) && (f=(Py_NB_FLOAT(arg)? NB_FLOAT(arg): NULL))))
        return PyTypeError("%s object can't be interpreted as a float", 
                           Py_TP_NAME(arg)), 0;
    if(PyFloat_Check(f)) {
        *d = PyFloat_AS_DOUBLE(f);
        Py_DECREF(f);
        return 1;
    }
    Py_DECREF(f);
    return PyTypeError("__float__ method of %s returned non-float '%s'",
                       Py_TP_NAME(arg), Py_TP_NAME(f)), 0;
}
BOOL_FUNC(py_int_as_sizes_t)(py_int *arg, sizes_t *v, const int ch) {
    //'ch'=N for size_t or 'n' for ssize_t
    // always checks for overflow
    if(!Py_SIZE(arg))
        return (v->i=0), TRUE;
    ssize_t auto size;
    digit const *p = arg->ob_digit;
    switch(ch) {
        case 'n': 
            if(SET_SIZE_AND_OVERFLOWED(ssize_t, p, size, Py_SIZE(arg)))
                return OVERFLOWS_AS(ssize_t), FALSE;
            break;    
    case 'N':
            if(SET_SIZE_AND_OVERFLOWED(size_t, p, size, Py_SIZE(arg)))
                return OVERFLOWS_AS(size_t), FALSE;
            break;    
    default:
        return PySystemError("invalid 'ch' for py_int_as_sizes_t"), FALSE;
    }
    for(v->u = p[--size]; size; ) {
        v->u <<= PyLong_SHIFT;
        v->u |= p[--size];
    }
    if(Py_SIZE(arg)<0)
        v->i = -v->i;
    return TRUE;
}
BOOL_FUNC(py_int_as_digit_t) (py_int *arg, digit_t *v, bint overflow) {
    /*
    Set the value in *v to the value of a python int with only one digit.
    Returns FALSE if overflow is FALSE and the int can't fit in 1 digit.
    */
    ssize_t size = Py_SIZE(arg);
    bint sign = TRUE;
    if(is_neg_ssize_t(size))
        size = -size;
    else 
        sign = FALSE;
    if(size>1)
        return PyOverflowError("Python can't fit in single digit"), FALSE;
    v->u = *(arg->ob_digit);
    if(sign)
        v->i = -v->i;
    return TRUE;
}
UTIL_FUNC(py_int_from_digits)(digit *p, ssize_t n) {
    bint const sign = n<0;
    size_t size = (size_t)(sign? -n: n);
    //beware of the empty loop
    while(size && !p[--size]);
    size = (!size && !p[0])? 0: size+1;
    return py_long_new(size, p, sign); 
}
FAST_FUNC(Py) py_int_from_byte(byte_t *byte, const int ch) {
    #define C_BYTE(b) ((byte_t*)(&(b)))
    switch(ch) {
        case 'u': return Py_NEWREF(SMALL_INTS[byte->u]);
        case 'i': return Py_NEWREF(SMALL_INTS[byte->i]);
    }
    return PySystemError("invalid fmt spec for py_int_from_byte");
}
FAST_FUNC(Object ) py_int_from_digit(digit d, int sign) {
    return py_long_new(1, &d, sign);
}
#define BLANK_INT(size) ((py_int*)(__var_object__(&PyLong_Type, (size))))
//#define BLANK_INT(size) (__var_object__(&PyLong_Type, size)).any
FAST_FUNC(Object ) py_int_from_ssize_t(ssize_t n) {
    SIZE_MASKS;
    py_alias_t r;
    sizes_t v;
    bool const s = n < 1;
    if(s) {
        if(n > -257)
            Py_RETURN_SMALL(n);
        v.u = (ssize_t)-n;
        goto A;
    }
    if(n < 257)
        Py_RETURN_SMALL(n);
    v.i = n;    
    A:
    ssize_t const ndigits = NDMASKS[2] & v.u? 3: (NDMASKS[1] & v.u? 2: 1);
    // Not really sure what's going on here with _PyLong_New.
    // Usually, 1-3% slower than __var_object__ then suddenly it's 10% faster
    //r = __var_object__(&PyLong_Type, ndigits);
    r.int_ = _PyLong_New(ndigits);
    if(!r.any)
        return NULL;
    int i = 0;
    switch(ndigits) {  
        case 3: 
            r.int_->ob_digit[i++] = (digit)(v.u & PyLong_MASK);
            v.u >>= PyLong_SHIFT;
        case 2:
            r.int_->ob_digit[i++] = (digit)(v.u & PyLong_MASK);
            v.u >>= PyLong_SHIFT;
        default:
            r.int_->ob_digit[i] = (digit)(v.u & PyLong_MASK);
    }
    if(s)
        r.var->ob_size = - r.var->ob_size;
    return r.ob;
}
FAST_FUNC(Object ) py_int_from_size_t(size_t n) {
    SIZE_MASKS;
    py_int *r;
    if(n < 257)
        Py_RETURN_SMALL(n);
                            
    ssize_t const ndigits = NDMASKS[2] & n? 3: (NDMASKS[1] & n? 2: 1);
    r = _PyLong_New(ndigits);
    if(!r)
        return NULL;
    // duff's device not actually helping here...
    for(size_t i=0, v=n;  v;  ++i,  v>>=PyLong_SHIFT)
        OB_DIGITS(r)[i] = (digit)(v & PyLong_MASK);
    return (Object)r;
}
FAST_FUNC(Object ) py_int_from_sizes_t(sizes_t *n, int const ch) {

    if(ch=='n')
        return py_int_from_ssize_t(n->i);    
    return py_int_from_size_t(n->u);
}
FAST_FUNC(Object ) py_int_from_rand64_t(rand64_t a) {
    /*
    For simplicity, this function acts as if a is an signed integer.
    
    On 32 bit builds if the rand64_t was initialized as a negative ssize_t, 
    it must be cast to int64_t before converting with this function so that
    the high 32-bits are non-zero and it can recognize it as a negative number.
    */
    NDMASKS_DEF;
    if(a.u < 257)
        return Py_NEWREF(SMALL_INTS[a.n]);
    int s=0, d=DPR;
    if(a.sign.signed_64) {
        if((s=(a.u = ((uint64_t)-a.i)) < 257))
            return Py_NEWREF(SMALL_INTS[-a.n]);
        s = 1;
    }
    A: 
    if(a.u & NDMASKS[d]) 
        goto B;
    else {
        --d; 
        goto A;
    }
    B: 
    ++d;
    py_int *v = _PyLong_New(d);
    digit *p = v->ob_digit;
    switch(d) {
        #ifdef RAND32
        case 5:
            (*p=(digit)(a.u &PyLong_MASK)), ++p, a.u >>= PyLong_SHIFT;
        case 4:
            (*p=(digit)(a.u &PyLong_MASK)), ++p, a.u >>= PyLong_SHIFT;
        #endif
        case 3:
            (*p=(digit)(a.u &PyLong_MASK)), ++p, a.u >>= PyLong_SHIFT;
        case 2:
            (*p=(digit)(a.u &PyLong_MASK)), ++p, a.u >>= PyLong_SHIFT;
        default:
            (*p=(digit)(a.u &PyLong_MASK));
    }
    if(s)
        Py_SIZE(v) = -Py_SIZE(v);
    return (Py)v;
}
FAST_FUNC(Object ) py_int_from_number(Object n) {
    /*
    Attempt to cast arbitrary object to python int, using either 
    nb_int nb_index (in that order) if it isn't already at least
    at least a subclass.    
    */
    
    Object i;
    i = (PyLong_CheckExact(n)? 
        Py_NEWREF(n)         :
        (PyLong_Check(n)     ? py_int_copy(n): NULL));
    if(i) return i;

    if(!Py_TP_NUMBER(n))
        goto X;
    if(!(i = Py_NB_INT(n)? NB_INT(n): (Py_NB_INDEX(n)? NB_INDEX(n): NULL)))
        goto X;
    if(PyLong_CheckExact(i))
        return i;
    if(PyLong_Check(i))
        return py_int_copy(i);
    Py_DECREF(i);
    return PyTypeError("integer coercion method of type '%s' returned "
                       "non-int (type '%s')",  Py_TP_NAME(n), Py_TP_NAME(i));  
  X:
    if(PyErr_Occurred())
        return NULL;
    if(i != NULL)
        return PySystemError("index shoulda been null...");
    return PyTypeError("unable to convert '%s' object to python int", 
                        Py_TP_NAME(n));  
}

FUNC(Object ) py_index(Object index) {               
    /*
    Sets error if object cannot be converted to an "index", oterhwise
    return a new reference to a PyLongObject.
    
    An "index" is an object with an __index__ method that must return 
    PyLongObject. Subclasses of python ints are "downcasted" to py_int.
    */
    Object integral;
    if(!index)
        return NULL;
    if(PyLong_CheckExact(index))
        return Py_NEWREF(index);
    if(PyLong_Check(index))
        return _PyLong_Copy((py_int*)index);
    if(!Py_TP_NUMBER(index))
        goto not_int;
    if(Py_TP_NUMBER(index)->nb_index)
        integral = Py_TP_NUMBER(index)->nb_index(index);
    else
        goto not_int;  
    if(!integral)
        return NULL;
    if(PyLong_CheckExact(integral))
        return integral;
    if(!PyLong_Check(integral))
        goto not_int;
    index = integral;
    integral = py_int_copy(index);
    Py_DECREF(index);
    return integral;
  not_int:
    return PyTypeError("'%s' object is not an integer",
                        Py_TP_NAME(index));
    
}
#ifdef RAND_DEBUG
    FAST_FUNC(bool) _validate_seq(PySequence* sq) {
        if(!pyseq_SQ(sq))
            return PySystemError("invalid PySequence: NULL sq slot"), 0;
        if(!pyseq_SQ_M(sq))
            return PySystemError("invalid PySequence: NULL sq_m slot"), 0;
        if(!pyseq_SQ_ITEM(sq))
            return PySystemError("invalid PySequence: NULL sq_item slot"), 0;
        if(!pyseq_SQ_M(sq)->sq_length)
            return PySystemError("invalid PySequence: NULL sq_m.sq_length slot"), 0;
        if(pyseq_SSIZE(sq)<0)
            return PySystemError("invalid PySequence: size less than 0 (%i)", 
                                (int)pyseq_SSIZE(sq)), 0;
        return 1;
    }
#   define py_seq_parse(ob, sq, unpack) \
        (_py_seq_parse((ob), (sq), (unpack)) && _validate_seq((sq)))
#else
#   define py_seq_parse _py_seq_parse
#endif

#define _(o) (Py_TYPE(o)->tp_as_sequence)
#define _is_sequence(o) (_o(o) && _o(o)->sq_length && _o(o)->sq_item)
FAST_FUNC(void*)   PySequence_init(PySequence *sq, Object it, bool unpack) {
#ifdef RAND_DEBUG
    if(!sq)
        return PySystemError("PySequence_init: got NULL sq");
    if(!(pyseq_SQ(sq)=it))
        return PySystemError("PySequence_init: got NULL iterable");
#endif
    PySequenceMethods *const m = Py_TP_SEQUENCE(it);
    bool const is_iter = unpack && (Py_TP_ITER(it) != NULL);
    bool const is_seq  = m && m->sq_length && m->sq_item;
    if(is_seq || is_iter)
        return  it;
    switch(unpack) {
        case 1:
            return PyTypeError("%s object is not iterable", pyseq_NAME(sq));
    }
    return PyTypeError("%s object is not a sequence", pyseq_NAME(sq));    
}
FAST_FUNC(Object)  PySequence_item(PySequence *sq, ssize_t const index) {
    
    Object result = NULL; 
    
#ifdef RAND_DEBUG
    if(pyseq_SQ_ITEM(sq)==NULL)
        return PySystemError("no sq_item for '%s' object",  
            Py_TP_NAME(sq->base.sq));
    int const i = pyseq_GET_ITEM(sq, index, &result);
    switch(i) {
    case FALSE: 
        return result;
    case TRUE:
        if(PyErr_Occurred())
            PySystemError("PySequence_item: index out of range but an error "
            " was already set");
        else
            PyIndexError("PySequence_item: %s index out of range", 
                pyseq_NAME(sq));
        break;
    case NEG1:
        if(!PyErr_Occurred())
            PySystemError("PySequence_item failed to propagate error");
        break;
    default:
        if(PyErr_Occurred())
            PySystemError("PySequence_item did not return 0, 1, or -1 and it"
            " already had an error set");
        else
            PySystemError("PySequence_item did not return a 0, 1, or -1");
        result = NULL;
    }
#else 
    int const i = pyseq_GET_ITEM(sq, index, &result);
    if(i)
#endif
        Py_XDECREF(result);
    return result;
}
BOOL_FUNC(fast_get_item)       (PySequence *sq, ssize_t index, Object *item) {
    if(index<0)
        index += pyseq_SSIZE(sq);
    if(index < 0 || index >=  pyseq_SSIZE(sq))
        return 1;
    if((*item = FastSequence_OB_ITEM(sq)[index]))
        return Py_NEWREF(*item), 0;
    return PySystemError("FastSequence ob_item contained a NULL value"), -1;
}
BOOL_FUNC(generic_get_item)    (PySequence *sq, ssize_t index, Object *item) {
    
    if(index < 0)
        index += pyseq_SSIZE(sq);
    if(index < 0 || index >=  pyseq_SSIZE(sq))
        return 1;
    Object ob = pyseq_SQ(sq);
    *item = Py_TYPE(ob)->tp_as_sequence->sq_item(ob, index);
    if(item==NULL)
        return -1;
    return 0;
}
BOOL_FUNC(range_get_item)      (PySequence *sq, ssize_t index, Object *item) {    

    if(index < 0)
        index += pyseq_SSIZE(sq);
    if(index > pyseq_SSIZE(sq))
        return 1;
    const ssize_t v = {sq->range.start + (sq->range.step * index)};
    return ((*item = py_int_from_ssize_t(v))? FALSE: NEG1);
}
BOOL_FUNC(latin1_get_item)     (PySequence *sq, ssize_t index, Object *item) { 
    if(index<0)
        index += sq->base.size;
    if(index<0 || index>= sq->base.size)
        return 1;
    uint8_t letter = sq->unicode.data.ucs1[index];
    if(!(*item = Py_NEWREF(LATIN1[letter]))) {
        PySystemError("LATIN1 character cache was deallocated");
        return -1;
    }
    return 0;
}
BOOL_FUNC(unicode_get_item)    (PySequence *sq, ssize_t index, Object *item) { 
    if(index<0)
        index += sq->base.size;
    if(index<0 || index>= sq->base.size)
        return TRUE;
    if(sq->unicode.maxsize==1)
        return (*item = Py_NEWREF(LATIN1[sq->unicode.data.ucs1[index]])), FALSE;  
    unicode_alias str;
    union {
        uint32_t ucs4;        
        uint16_t ucs2;
        uint8_t ucs1;
    } letter;
    switch(sq->unicode.maxsize) {
        case 2:
            letter.ucs2 = sq->unicode.data.ucs2[ index ];
            if(letter.ucs2 < 256)
                return (*item = Py_NEWREF(LATIN1[letter.ucs2])), FALSE; 
            str.as_object = PyUnicode_New(1, letter.ucs2);
            if(!(*item=str.as_object))
                return -1;
            *((uint16_t*)(str.as_compact+1)) = letter.ucs2;
            return FALSE;
        case 4:
            letter.ucs4 = sq->unicode.data.ucs4[index];
            if(letter.ucs4 < 256)
                return (*item = Py_NEWREF(LATIN1[letter.ucs4])), FALSE; 
            str.as_object = PyUnicode_New(1, letter.ucs4);
            if(!(*item=str.as_object))
                return -1;
            //in case this is an issue on fo big endian?
            if(letter.ucs4 < 65536)
                *((uint16_t*)(str.as_compact+1)) = letter.ucs2;
            *((uint32_t*)(str.as_compact+1)) = letter.ucs4;
            return FALSE;
    }
    return PySystemError("unicode.maxsize was not 2 or 4..."), NEG1;
}
BOOL_FUNC(py_seq_parse_range)  (py_range *range, PySequence *sq) {   
    if(  !(py_int_as_sizes_t(range->length, pyseq_SIZES(sq), 'n')
        && py_int_as_sizes_t(range->start, (sizes_t*)&RangeSeq_START(sq), 'n')
        && py_int_as_sizes_t(range->step, (sizes_t*)&RangeSeq_STEP(sq), 'n')))
        return FALSE; 
    Py_INCREF(pyseq_SQ(sq));
    pyseq_SQ_ITEM(sq) = range_get_item;  
    return TRUE; 
}
BOOL_FUNC(py_seq_parse_list)   (py_list* it, PySequence *sq) {
    FastSequence_OB_ITEM(sq) = pyseq_SQ_AS_LIST(sq)->ob_item;
    pyseq_SSIZE(sq)          = Py_SIZE(it);
    pyseq_SQ_ITEM(sq)        = fast_get_item;
    Py_INCREF(it);
    return TRUE;
}
BOOL_FUNC(py_seq_parse_tuple)  (py_tuple* it, PySequence *sq) {
    FastSequence_OB_ITEM(sq) = pyseq_SQ_AS_TUPLE(sq)->ob_item;
    pyseq_SSIZE(sq)          = Py_SIZE(it);
    pyseq_SQ_ITEM(sq)        = fast_get_item;
    Py_INCREF(it);
    return TRUE;
}
BOOL_FUNC(py_seq_parse_iter)   (Object it, PySequence *sq) {    
    if(!(pyseq_SQ(sq)=PySequence_List(it)))
        return FALSE;
    FastSequence_OB_ITEM(sq) = pyseq_SQ_AS_LIST(sq)->ob_item;
    pyseq_SSIZE(sq)          = Py_SIZE(pyseq_SQ(sq));
    pyseq_SQ_ITEM(sq)        = fast_get_item;
    return TRUE;
}
BOOL_FUNC(py_seq_parse_unicode)(UnicodeRepr *unicode, PySequence *sq) {
    py_alias_t u = {.repr = unicode};
    #define state u.repr->state
    Py_INCREF(unicode);
    if(!state.compact) {
        sq->base.size = PyObject_Size(u.ob);
        sq->base.sq_item = generic_get_item;
        goto END;
    }
    sq->base.size = u.repr->length;
    sq->unicode.maxsize = state.kind;
    if(state.ascii) {
        sq->unicode.data.any = (void*)(u.as_ascii+1);
        sq->base.sq_item = latin1_get_item;
        goto END;
    }
    if(state.kind==1) {
        sq->unicode.data.any = (void*)(u.as_compact+1);
        sq->base.sq_item = latin1_get_item;
        goto END;
    }
    sq->unicode.data.any = (void*)(u.as_compact+1);
    sq->base.sq_item = unicode_get_item;
    END:
    UnicodeSeq_DLEN(sq) = sq->base.size * state.kind;
    //UnicodeSeq_DLEN(sq) = sq->base.size * u.repr->state.kind;
    return TRUE;   
    #undef state
}
BOOL_FUNC(py_seq_parse_generic)(Object ob, PySequence *sq) {
    //covers all other "sequences" besides lists, tuples, ranges, and strings
    if((pyseq_SSIZE(sq) = PyObject_Size(ob)) < 0)
        return FALSE;
    pyseq_SQ_ITEM(sq) = generic_get_item;
    Py_INCREF(pyseq_SQ(sq));
    return TRUE;
}
BOOL_FUNC(py_seq_parse_set)    (Object ob, PySequence *sq) {

    pyseq_SQ(sq) = PyList_New((pyseq_SSIZE(sq)=PySet_GET_SIZE(ob)));
    Py x = NULL;
    FastSequence_OB_ITEM(sq) = pyseq_SQ_AS_LIST(sq)->ob_item;
    for(ssize_t i=0, j=0, h=0; _PySet_NextEntry(ob, &j, &x, &h); ++i)
       if(!(x? (FastSequence_OB_ITEM(sq)[i] = Py_NEWREF(x)): 0))
            goto fail;    
    pyseq_SQ_ITEM(sq)        = fast_get_item;
    return 1;
  fail:
    Py_DECREF(pyseq_SQ(sq));
    return PySystemError("cosmic ray error"),0;
}
BOOL_FUNC(py_sequence_parse) (Object ob, PySequence *sq, bool unpack) { 
    
    if(!sq)
        return PySystemError("PySequence_init: got NULL sq"), 0;    
    
    if(!ob)
        return PySystemError("PySequence_parse: NULL ob"), 0;
    
    pyseq_SQ(sq) = ob;
    
    if(!(Py_TP_SEQUENCE(ob) && Py_SQ_LENGTH(ob) && Py_SQ_ITEM(ob)))
        goto not_seq;

    if(PyList_Check(ob))
        return py_seq_parse_list((py_list*)ob, sq);
        
    if(PyTuple_Check(ob))
        return py_seq_parse_tuple((py_tuple*)ob, sq);
        
    if(PyUnicode_Check(ob))
        return py_seq_parse_unicode((UnicodeRepr*)ob, sq);
        
    if(PyRange_Check(ob))
        return py_seq_parse_range((py_range*)ob, sq);
    
    return py_seq_parse_generic(ob, sq);
    
  not_seq:
    
    if(!unpack)
        return PyTypeError("'%s' object is not a sequence", pyseq_NAME(sq)), 0;
    
    if(PyAnySet_Check(ob))
        return py_seq_parse_set(ob, sq);
        
    if(Py_TP_ITER(ob))
        return py_seq_parse_iter(ob, sq);
    return PyTypeError("'%s' object is not iterable", pyseq_NAME(sq)), 0;
}
#define py_int_as_rand64_t(v) {.u=py_int_as_uint64_t(v)}

FAST_FUNC(int) rand_fmod(double a, double b, double *mod) {
    if(!b)
        return 0;
    *mod = fmod(a, b);
    if(*mod)
        return (*mod= (((b < 0) != (*mod < 0))? *mod+b: *mod)), 1;
    return copysign(0.0, b), 1;
}
BOOL_FUNC(py_list_parse_floats) (Object vals, double *mem, double *sum) {
    PyObject *val;
    double prev, next;

    size_t const size = (size_t)Py_SIZE(vals);

    if(!mem) {
        if(PyErr_Occurred)
            return FALSE;
        return PyErr_NoMemory(), FALSE;
    }
    mem[0] = (*sum=FALSE), (prev=FALSE), (next=FALSE);

    for(size_t i=0; i<size; ++i) {
        
        val = LIST_ITEMS(vals)[i];
        if(!PyFloat_Check(val))
            return PyTypeError(no_homo_d, Py_TP_NAME(val)), FALSE;
       
        next = PyFloat_AS_DOUBLE(LIST_ITEMS(vals)[i]);
        if(!Py_IS_FINITE(next))
            return PyValueError(bad_wflote), FALSE;
        
        *sum += next;
        if(!(*sum > prev && Py_IS_FINITE(*sum)))
            return PyValueError(zero_w), FALSE;
        mem[i] = *sum;         
        prev = mem[i];
    }
    return 1;
}
BOOL_FUNC(py_list_parse_size_t) (Object vals, size_t *mem, size_t *sum) {   
    PyObject *val;
    size_t prev;
    sizes_t next;
    size_t const size = (size_t)Py_SIZE(vals);

    if(!mem) {
        if(PyErr_Occurred)
            return FALSE;
        return PyErr_NoMemory(), FALSE;
    }
    *sum = 0, prev=0, next.i=0;
    
    for(size_t i=0; i<size; ++i) {
        
        val = LIST_ITEMS(vals)[i];

        if(!PyLong_Check(val))
            return PyTypeError(no_homo_i, Py_TP_NAME(val)), FALSE;
        if(!py_int_as_sizes_t((py_int*)val, &next, 'N'))
            return FALSE;
        if(next.i < 1)
            return PyValueError(zero_w), FALSE;
        
        *sum += next.u;
        if(*sum <= prev)
            return PyValueError(bad_wint), FALSE;
        mem[i] = *sum;         
        prev = mem[i];
    }
    return 1;
}
FUNC(size_t) bisect_double(double *a, double const v, size_t hi) {
    //the first index in an array *a such that v>=a[index]
    size_t lo = 0;
    for(size_t mid; lo < hi; ) {
        mid = (lo + hi) / 2;
        if(v < a[mid])
            hi = mid;
        else 
            lo = mid + 1;
    }
    return lo;
}
FUNC(size_t) bisect_size_t(size_t *a, size_t const v, size_t hi) {
    //the first index in an array *a such that v>=a[index]
    size_t lo = 0;
    for(size_t mid; lo < hi; ) {
        mid = (lo + hi) / 2;
        if(v < a[mid])
            hi = mid;
        else 
            lo = mid + 1;
    }
    return lo;
}
FUNC(int) no_kwargs(const char *f, Py kws) {
    if(kws) {
        if(!PyDict_CheckExact(kws))
            PySystemError("'no_kwargs(%s) because object wasn't dict", f);
        else if(PyDict_GET_SIZE(kws) != 0)
            PyTypeError("%.200s() takes no keyword arguments", f);
        else 
            return 1;
        return 0;
    }
    return 1;
}
FUNC(int) UnicodeSeq_memcpy(Py unicode, char *buf) {
    
    PySequence sq = PySequence_new();
    
    if(!py_sequence_parse(unicode, &sq, 0))
        return 0;
    memcpy(buf, UnicodeSeq_DATA(&sq).ucs1, (size_t)UnicodeSeq_DLEN(&sq));
    PySeq_CLEAR(&sq);
    return 1;
}
FUNC(int) double_repr(double v, char *buf) {
    #define DOUBLE_BUFSIZE 64
    #define SETUP_DOUBLE_BUF(name) char name[DOUBLE_BUFSIZE] = {0}; 
    char *fbuf;
    fbuf = osfunc(double_to_string)(v, 'r', 0, Py_DTSF_ADD_DOT_0, NULL);
    if(!fbuf)
        return PyErr_NoMemory(), 0;
    memcpy(buf, fbuf, DOUBLE_BUFSIZE);
    PyMem_Free(fbuf);
    return 1;
}
FUNC(int) sizes_repr(sizes_t const *v, char *buf, const int ch) {

    #define SIZES_BUFSIZE 32
    #define SETUP_SIZES_BUF(name) char name[SIZES_BUFSIZE] = {0};
    
    const char *bases[] = {"%zi", "%zu"};
    switch(ch) {
        case 'i': return str_format(buf, SIZES_BUFSIZE, bases[0], v->i);
        case 'u': return str_format(buf, SIZES_BUFSIZE, bases[1], v->u);
        default: return PySystemError("invalid ch in sizes_repr"), 0;
    }
}
FUNC(ssize_t) call_repr(char **repr_buf, const char *name, const char *params){
    sizes_t length;
    char *buf;
    length.u = strlen(name);
    length.u += 8 + strlen(params);
    buf = PyMem_Malloc(length.i);
    if(!buf)
        return PyErr_NoMemory(), -1;
    *repr_buf = buf;
    int r = str_format(buf, length.u, "%s(%s)", name, params);
    if(r < 0)
        PyMem_Free(buf);
    length.i = (ssize_t)r;
    return length.i;
}
#include "argsparse.h" 
void set_clear_entries(rset *so) {
    size_t i;
    size_t const num_keys = so->mask+1;
    for(i=0; i<so->size; ++i) {
        so->data[i].key = -1;
        so->data[i].next = NULL;
    }
    for(i=0; i<num_keys; ++i) {
        so->table[i] = NULL;
    }
}
FUNC(Py)       setobject_new(PyTypeObject *, Py, Py);
rset *new_set(sizes_t *);
FUNC(size_t*) getsizeof_rset(sizeof_rset_args *s) {
    #ifdef RAND_DEBUG
    if(!s->size)
        return (void*)PySystemError("sizeof_rset_args: didn't set size field");
    #endif
    s->num_slots      = *s->size + 1; //+1 sentinel slot
    size_t const keys = (size_t)1 << size_t_bit_length(s->num_slots);
    size_t const load = *s->size * 100 / keys;
    s->num_keys       = load>60? keys<<1: keys;
    s->data_offset    = sizeof(rset) + sizeof(set_entry**) * s->num_keys;
    s->data_size      = sizeof(set_entry) * s->num_slots;
    return s->size;
}
FUNC(Py) setobject___sizeof__(setobject *so) {
    sizeof_rset_args s = {&so->set->size};
    getsizeof_rset(&s);
    return py_int_from_size_t(s.data_offset + s.data_size);
}
rset *new_set(sizes_t *size) {
    
    rset *so;
    sizeof_rset_args s = {&size->u};
    union {  
        void *any;
        char *buf;
        rset *so;
     } view;
     
    getsizeof_rset(&s);

    view.any = PyObject_Malloc(s.data_offset + s.data_size);
    if(!view.any)
        return (void*)PyErr_NoMemory();   
        
    so        = view.so;
    so->size  = size->i;
    so->mask  = s.num_keys - 1;
    so->used  = 0;
    
    so->table = (set_entry**)(view.buf + sizeof(rset));
    so->data  = (set_entry *)(view.buf + s.data_offset);
    
    set_clear_entries(so);
    so->data[size->i].key = -1;
    so->data[size->i].next = NULL;
    
    return so;
}
FUNC(Py)       setobject_new(PyTypeObject *tp, Py args, Py kwargs) {
    setobject *so = NULL;
    rset *seto;
    DEFINE_VARARGS(indices, 1, 'n') = {"size"};
    VARARG(sizes_t, size, {0});
    PARSE_VARARGS();
    
    if(size.i < 1)
        return PyValueError("size must be greater than 0");
    seto = new_set(&size);
    so = __object__(&NumberSet_Type);
    if(!so)
        return NULL;
    so->set = seto;
    return (Py)so;
}
ssize_t set_get_entry(rset *so, sizes_t *key, set_entry **next_free) {
    
    /* will return and set *next_free to one of these pairs:
      hash  free
    a:  -1, !NULL -> key already present
    b:  -1,  NULL -> cosmic ray error (should never happen)
    c: !-1,  NULL -> key goes first slot
    d: !-1, !NULL -> collision; key not in first slot 
    */
    set_entry *entry;
    sizes_t hash;
    
    hash.i = key->i==-1? -2: key->i;
    hash.u &= so->mask;
    
    entry      = so->table[hash.u];    
    *next_free = NULL;
    if(!entry)      
        return hash.i;
    if(entry->next) {
        do {
            if(entry->key == key->i)
                return -1;
            entry = entry->next;
        } while(entry->next);
    }
    if(entry->key == key->i)
        return -1;
    *next_free = entry;
    return hash.i;    
}
FUNC(int) set_contains(rset *so, void *key) {
    set_entry *entry;
    return set_get_entry(so, (sizes_t*)key, &entry) == -1;
}
FUNC(int) set_add_entry(rset *so, void *key) {
    
    set_entry *entry;
    
    if(so->used == so->size)
        return -1;
    const sizes_t hash ={ set_get_entry(so, (sizes_t*)key, &entry)};
    if(hash.i == -1)
        return 0;

    if(entry==NULL)
        so->table[hash.i] = (entry = &so->data[so->used]);
    else
        entry = (entry->next = &so->data[so->used]);
    entry->key = ((sizes_t*)key)->i;
    ++so->used; 
    return 1;
}

VOID_FUNC(setobject_dealloc)(setobject *so) {
    PyObject_Free(so->set);
    PyObject_Del(so);
}
FUNC(Py)  setobject_mask(setobject *so) {
    return py_int_from_size_t(so->set->mask);
}
FUNC(Py)  setobject_update(setobject *so, Py ob) {
    Py r;
    size_t free;
    Py t;
    sizes_t key;
    PySequence s = PySequence_new();
    PySequence *sq = &s;
    if(!py_sequence_parse(ob, sq, 1))
        return NULL;
    if(pyseq_SIZE(sq) > (free=(so->set->size - so->set->used)))      
        goto Y;

    for(size_t i=0; i<pyseq_SIZE(sq) ; ++i){
        t = PySequence_item(sq, i);
        if(!t) {
            r = NULL;
            goto X;
        }
        if(!py_int_as_sizes_t((py_int*)t, &key, 'n')){
            r = NULL;
            goto X;
        }
        if(set_add_entry(so->set, &key)<0)
            goto Y;;
    }
    r = Py_NEWREF(Py_None);
    X:
    PySeq_CLEAR(sq);
    return r;
    Y:
    r = PyValueError("set is full");
    goto X;
}
FUNC(Py)  setobject_add(setobject *so, Py ob) {
    sizes_t key;
    if(!PyLong_Check(ob))
        return PyTypeError("can only add integers");
    if(!py_int_as_sizes_t((py_int*)ob, &key, 'n'))
        return NULL;
    set_add_entry(so->set, &key);
    prn;
}
FUNC(Py)  setobject_iter(setobject *so) {
    setiter *si = __object__(&setiter_type);
    if(!si)
        return NULL;
    si->set = (setobject*)Py_NEWREF(so);
    si->i = 0;
    return (Py)si;
}
SSIZEFUNC(setobject_len)(setobject *so) {
    return (ssize_t)so->set->used;
}
FUNC(int) setobject_contains(setobject *so, Py ob) {
    sizes_t elem;
    if(!PyLong_Check(ob))
        return 0;
    if(!py_int_as_sizes_t((py_int*)ob, &elem, 'n'))
        return -1;
    return set_contains(so->set, &elem);
}
FUNC(Py)  setobject_repr(Py so) {
    Py reprlist, reprrep, reprslice, r;
    reprlist = PySequence_List(so);
    if(!reprlist)
        return NULL;
    if(!(reprrep = PyList_Type.tp_repr(reprlist)))
        return NULL;
    if(!(reprslice = PySequence_GetSlice(reprrep, 1, -1)))
        return NULL;
    r = PyUnicode_FromFormat("NumberSet({%S})", reprslice);

    Py_XDECREF(reprrep);
    Py_XDECREF(reprslice);
    Py_XDECREF(reprlist);
    return r;
}

FUNC(Py)  setiter_next(setiter *si) {
    Py r;
    setobject *set = si->set;
    rset *so = set->set;
    size_t size = so->size;
    if(si->i >= si->set->set->used)
        return NULL;
    r = py_int_from_size_t(so->data[si->i].key);
    ++si->i;
    return r;
}
FUNC(Py)  setiter_iter(setiter *si) {
    return Py_NEWREF((Py)si);
}
VOID_FUNC(setiter_dealloc)(setobject *si) {
    Py_DECREF(si->set);
    PyObject_Del(si);
}
FUNC(int) py_seq_sum_doubles(PySequence *sq, double *a) {
    Py t;
    double f;
    for(ssize_t i=0; i<pyseq_SSIZE(sq); ++i) {    
        if(!(t = PySequence_item(sq, i)))
            return 0;
        if(!(py_number_as_double(t, &f))) {
            Py_DECREF(t);
            return 0;
        }
        *a += f;
        Py_DECREF(t);
    }
    return 1;         
}
FUNC(Py) py_stdev(Py m, Py arg) {
    
    int v;
    PySequence s = PySequence_new(), *sq = &s;
    Object t=NULL;
    double f=0.0, mean=0.0, sum=0.0;

    if(!py_sequence_parse(arg, sq, 1)) 
        return NULL;
    if(!py_seq_sum_doubles(sq, &f))
        goto X;
        
    double const length = (double)pyseq_SSIZE(sq);
    mean = f/length;
  
    for(ssize_t i=0; i<pyseq_SSIZE(sq); ++i) {    
        if(!(t = PySequence_item(sq, i)))
            goto X;
        v = py_number_as_double(t, &f);
        if(!v)
            goto X;    
        sum += pow(f-mean, 2.0);
        Py_DECREF(t);
    }
    return __float__(sqrt(sum/length));
    X:
    PySeq_CLEAR(sq);
    Py_XDECREF(t);
    return NULL;
}
