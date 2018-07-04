
#ifndef XRAND_TYPES_H
#define XRAND_TYPES_H
#define _QUALNAME_STR1_IMPL_(x) #x
#define _QUALNAME_STR1_(x) _QUALNAME_STR1_IMPL_(x)
#define _QUALNAME_IMPL_(tp) _QUALNAME_STR1_(_MODULE_.tp)
#define QUALNAME(tp) _QUALNAME_IMPL_(tp)

#define OB_STRUCT_ID_U(tp) _ID_IMPL_(tp, Object)
#define OB_STRUCT_ID_L(tp) _ID_IMPL_(tp, object)
#define INTERNAL_TYPE_IDENT(tp) _ID_IMPL_(tp, _NAME)
#define _ID_IMPL_(x, name) _ID_IMPL_A_(_ID_CONCAT_(x, name))
#define _ID_IMPL_A_(x) x

#define _ID_CONCAT_(tp, name) _ID_CONCAT_IMPL_(tp, name)
#define _ID_CONCAT_IMPL_(tp, name) tp##name
#define TYPE(tp) typedef tp
#define HASH_FUNC(name) static Py_hash_t name
#define SIZE_FUNC(name) static size_t name
#define SSIZEFUNC(name) static ssize_t name
#define DATA(tp, ...) static tp __VA_ARGS__
#define FUNC(tp) static tp
#define FAST_FUNC(tp) static __inline tp
#define IMPL_FUNC(tp) static tp
#define TYPE_INIT(tp) static int tp##_init_type
#define RAND_METH(name) static PyObject *Random_##name
#define RAND_IMPL(name) static PyObject *random_##name##_impl
#define _c_(tp, ob) ((tp)(ob))


#define RANDOMOB OB_STRUCT_ID_U(Random)
#define RANDITER_OB OB_STRUCT_ID_L(randiter)
#define RANDITER_NAME randiter
#define POPENTRY_NAME PopEntry
#define POPULATION_NAME Population
#define CHOICES_NAME choices
#define DICE_NAME dice 
#define RANDOM_NAME Random
#define SET_NAME IntSet
#define prz return Py_NEWREF(SMALL_INTS[0])
#define prn Py_RETURN_NONE
#define str_format PyOS_snprintf
#define __format__ PyUnicode_FromFormat
#define __float__(o) (PyFloat_FromDouble((o)))
#define __list__(o) (PyList_New((o)))
#define __tuple__(o) (PyTuple_New((o)))
#define __dict__() (PyDict_New())
#define py_bytes PyBytesObject
#define py_dict PyDictObject
#define py_float PyFloatObject
#define py_int PyLongObject
#define py_list PyListObject
#define py_set PySetObject
#define py_tuple PyTupleObject
#define py_type PyTypeObject
#define py_unicode PyUnicodeObject
#define CHOICESSELF ChoicesObject *ro
#define RNGSELF RANDOMOB *rng
#define DICESELF diceobject *ro
#define msizeof(to, member) sizeof(((to *)0)->member)
#define Choices_Check(o) (Py_TYPE(o) == &Choices_Type)
#define PyType_HeapType(o) ((o)->tp_flags & Py_TPFLAGS_HEAPTYPE)
#define RandomDice_Check(o) (Py_TYPE(o) == &Dice_Type)
#define PopEntry_Check(o) (Py_TYPE(o) == &PopEntry_Type)
#define Population_Check(o) (Py_TYPE(o) == &Population_Type)
#define osfunc(func) PyOS_##func
#if !defined RandIter_RVs_Impl
/* NEEDS AT LEAST ONE VARARG*/
#define RV_ITER_SETUP(f, flag, ...) \
    static const randiter_funcs funcs = {\
        randiter_rv_next,\
        (reprfunc)randiter_##f##_repr,\
        NULL,\
        NULL,\
    };\
    randiter_setup_args a = {\
        .args     = args,\
        .defaults = {\
            .statf.impls.f = {__VA_ARGS__},\
            .statf.impl = flag\
        },\
        .funcs    = &funcs,\
        .unpacker = statf_unpackers,\
    };\
    return (Py)randiter_new(rng, &a)
#endif

#if PyLong_SHIFT==15
#   if SIZEOF_SIZE_T != 4
#       error SIZEOF_SIZE_T must be 4 when PyLong_SHIFT is 15
#   endif
#   define RAND32
#   define RAND32_ISDEF TRUE
#elif PyLong_SHIFT == 30
#   if SIZEOF_SIZE_T != 8
#       error SIZEOF_SIZE_T must be 8 when PyLong_SHIFT is 30
#   endif
#   define RAND32_ISDEF FALSE
#else
#   error PyLong_SHIFT must use one of the default values: 15 or 30.
#endif
#ifdef _MSC_VER
#   include <stdlib.h>
#   define ROTL _rotl64
#else
    static inline uint64_t ROTL(uint64_t const x, int const k) {
        return (x << k) | (x >> (64 - k));
    }
#endif
#define Dx 0xf000000000000000ULL
#define M3  0b00001111111111111110000000000000
#define M2  0b00000000000000000001111111111111
#define M1  0b00111111111111111000000000000000
#define M0  0b00000000000000000111111111111111
#define FAST_DIGITS2(dst, w) (\
    ((dst)[0]) = (((w[0] & M1) << 1) | ( w[0] & M0)),\
    ((dst)[1]) = ((w[0] >> 30) | ((w[1] & M2) << 2) | (( w[1] & M3) << 3))\
    )
#ifdef RAND32
#define DPR 4
#define BPD 2

#define D0 0x0000000000007fffULL
#define D1 0x000000003fff8000ULL
#define D2 0x00001fffc0000000ULL
#define D3 0x0fffe00000000000ULL
#define SIZE_MASKS static uint32_t NDMASKS[] = {0x7fffU, 0x3fff8000U,0xc0000000U}
#define NDMASKS_DEF static uint64_t NDMASKS[] = {D0,D1,D2,D3,Dx}
#else
#define BPD 4
#define DPR 2
#define D0 0x000000003fffffffULL
#define D1 0x0fffffffc0000000ULL
#define NDMASKS_DEF static uint64_t NDMASKS[] = {D0,D1,Dx}
#define SIZE_MASKS NDMASKS_DEF
#endif
enum _bint {NEG1=-1, FALSE, TRUE};
enum _data_sizes {
    TINY_INT_MIN=-256, 
    TINY_INT_MAX=256,
};
enum _data_quatities {
    TINY_INTS =  ((1 + TINY_INT_MAX) - TINY_INT_MIN),
    CACHED_CHARS = 256
};
DATA(PyTypeObject)Random_Type;
DATA(PyTypeObject)Dice_Type;
DATA(PyTypeObject)Choices_Type;
DATA(PyTypeObject)NumberSet_Type;
DATA(PyTypeObject)setiter_type;
DATA(PyTypeObject)PopEntry_Type;
DATA(PyTypeObject)Population_Type;
DATA(PyTypeObject)RandIter_Type;


TYPE(double const) c_double;
TYPE(char const)   c_str[];

TYPE(PyObject) *Py;
TYPE(PyObject) *Object;

TYPE(struct) RandomObject RandomObject;
TYPE(struct) diceobject diceobject;
TYPE(struct) DiceObject DiceObject;
TYPE(struct) ChoicesObject ChoicesObject;
TYPE(struct) setobject setobject;
TYPE(struct) setiter setiter;
TYPE(struct) rset rset;
TYPE(struct) set_entry set_entry;
TYPE(struct) randiter randiter;
TYPE(struct) randiter_funcs randiter_funcs;
TYPE(struct) randiter_setup_args randiter_setup_args;
TYPE(struct) FastDictKey FastDictKey;
TYPE(struct) GenericPySequence GenericPySequence;
TYPE(struct) FastSequence FastSequence;
TYPE(struct) RangeSequence RangeSequence;
TYPE(struct) UnicodeSequence UnicodeSequence;
TYPE(struct) ArgSpec ArgSpec;
TYPE(struct) py_range py_range;
TYPE(struct) UnicodeRepr UnicodeRepr;
TYPE(struct) signmap_t signmap_t;
TYPE(struct) unicode_state unicode_state;
TYPE(struct) py_long py_long;
TYPE(struct) py_dict_proxy py_dict_proxy;
TYPE(struct) Population Population;
TYPE(struct) popentry popentry;
TYPE(struct) sizeof_rset_args sizeof_rset_args;
TYPE(union) randiter_args_t randiter_args_t;
TYPE(struct) u_digit_t u_digit_t;
TYPE(struct) s_digit_t s_digit_t;
TYPE(union) ptr_t ptr_t;
TYPE(union) PySequence PySequence;
TYPE(union) unicode_data unicode_data;
TYPE(union) byte_t byte_t;
TYPE(union) unicode_alias unicode_alias;
TYPE(union) py_alias_t py_alias_t;
TYPE(union) digit_t digit_t;
TYPE(union) sizes_t sizes_t;
TYPE(union) rand64_t rand64_t;

TYPE(Py) (*riternext)(randiter*);
TYPE(Py) (*randiter_unpacker)(randiter *, randiter_args_t *, Py);
TYPE(int)(*argparser)(ArgSpec *, Py, ptr_t *, Py*);
struct py_long{
    PyObject_VAR_HEAD
    union {
        digit p;
        sdigit s;
        digit u_ob_digit[1];
        sdigit s_ob_digit[1];
    } ob_digit;
};
struct popentry{
    #define PopEntry_ITEM(o) (((popentry*)(o))->addr)
    #define PopEntry_BASE(o) (((popentry*)(o))->base)
    #define PopEntry_COMP(o) (((popentry*)(o))->cmp_eq)
    #define PopEntry_HASH(o) (((popentry*)(o))->hash)
    PyObject_HEAD
    Py addr;
    Py_hash_t hash;
    PyTypeObject *base;
    richcmpfunc cmp_eq;
};

struct Population {
    #define Population_MP(o) (((Population*)(o))->mp)
    PyObject_HEAD
    Py mp;
};
struct setobject {
    PyObject_HEAD
    rset *set;
};
struct setiter {
    PyObject_HEAD
    setobject *set;
    size_t i;
};

struct set_entry {
    ssize_t key;
    set_entry *next;
};
struct rset {
    size_t mask, size, used;
    set_entry  *data, **table;
};
struct py_range {
    PyObject_HEAD
    py_int *start;
    py_int *stop;
    py_int *step;
    py_int *length;
};

struct unicode_state  {
    unsigned int interned:2;
    unsigned int kind:3;
    unsigned int compact:1;
    unsigned int ascii:1;
    unsigned int ready:1;
    unsigned int :24;
} ;
union unicode_data{
    ssize_t  as_int;
    uint32_t letter;
    uint32_t *ucs4;
    uint16_t  *ucs2;
    uint8_t  *ucs1;
    void     *any;
} ;
struct UnicodeRepr{
    PyObject_HEAD
    ssize_t length;
    Py_hash_t hash;
    unicode_state state;
    wchar_t *pad1;
    unicode_data ascii_data;
    char *pad2;
    ssize_t pad3;
    unicode_data compact_data;
};
union byte_t{
    char i;
    char const ci;
    uint8_t u;
    uint8_t const cu;
    struct {
        uint8_t bit_0: 1;
        uint8_t bit_1: 1;
        uint8_t bit_2: 1;
        uint8_t bit_3: 1;
        uint8_t bit_4: 1;
        uint8_t bit_5: 1;
        uint8_t bit_6: 1;
        uint8_t bit_7: 1;
    } bits;
};
union unicode_alias{
    void *any;
    PyObject *as_object;
    UnicodeRepr *repr;
    PyUnicodeObject *as_unicode;
    PyCompactUnicodeObject *as_compact;
    PyASCIIObject *as_ascii;
};
struct ArgSpec {
    const char  *fname;
    const char **vnames;
    char       min_args;
    char       max_args;
    struct {
        char allow_none: 1;
        char :7;
        char pad;
    } flags;
    int const *allow_Nones;
    argparser  *parsers;
};

struct FastDictKey {
    void *key;
    Py_hash_t hash;
    const char *str;
};
struct py_dict_proxy {
    PyObject_HEAD
    PyObject *mp;
};
struct GenericPySequence {
    PyObject *sq;
    ssize_t size;
    int (*sq_item) (PySequence *, ssize_t, PyObject **);
};
struct FastSequence {
    GenericPySequence base;
    PyObject **ob_item;
};
struct RangeSequence {
    GenericPySequence base;
    ssize_t start;
    ssize_t step;
};
struct UnicodeSequence {
    GenericPySequence base;
    int maxsize;
    unicode_data data;
    ssize_t dlen;
};
struct sizeof_rset_args {
    size_t *size, num_slots, num_keys, data_offset, data_size;
};

union PySequence {
    GenericPySequence base;
    FastSequence fast;
    RangeSequence range;
    UnicodeSequence unicode;
};
union py_alias_t{
    void *any;
    RandomObject *rand;
    PyObject *ob;
    UnicodeRepr *repr;
    PyUnicodeObject *as_unicode;
    PyCompactUnicodeObject *as_compact;
    PyASCIIObject *as_ascii;
    PyVarObject *var;
    py_bytes *bytes;
    py_dict *dict;
    py_int *int_;
    py_long *int_repr;
    py_list *list;
    py_range *range;
    py_tuple *tuple;
    py_type *type;
    char *data;
};
union digit_t{
    digit u;
    sdigit i;
    digit p[1];
    sdigit q[1];
    struct {
        digit:sizeof(digit) * 8-1;
        digit is_signed: 1;
    } sign;
};

#ifdef RAND32


struct u_digit_t {
    uint64_t d0:15;
    uint64_t d1:15;
    uint64_t d2:15;
    uint64_t d3:15;
    uint64_t dx: 4;
};
struct s_digit_t {
    int64_t d0: 15;
    int64_t d1: 15;
    int64_t d2: 15;
    int64_t d3: 15;
    int64_t dx: 4;
};
#else
struct u_digit_t {
    uint64_t d0:30;
    uint64_t d1:30;
    uint64_t dx: 4;
};
struct s_digit_t {
    int64_t d0: 30;
    int64_t d1: 30;
    int64_t dx: 4;
};
#endif
struct signmap_t {
    uint8_t:7;
    uint8_t signed_8:1;
    uint8_t:7;
    uint8_t signed_16:1;
    uint16_t:15;
    uint16_t signed_32:1;
    uint32_t:31;
    uint32_t signed_64:1;
} ;
union sizes_t {
    size_t u;
    ssize_t i;
    struct {
        size_t msb: 1;
        #ifdef RAND32
        size_t :31;
        #else
        size_t : 63;
        #endif
    } sign;
};
union rand64_t {
    int64_t      i;
    uint64_t     u;
    ssize_t      n;
    size_t       N;
    sizes_t      c;
    double       d;
    uint8_t    str[8];
    digit        p[DPR];
    digit        h;
    twodigits    T;
    uint32_t     w[2];
    digit_t     pq;
    u_digit_t   ud;
    s_digit_t   sd;
    signmap_t sign;
};
union ptr_t {
    void *any;
    int *i;
    double *d;
    rand64_t *v;
    PyObject **o;
    py_alias_t *py;
    digit_t *pq;
    const char **str;
    PySequence *sq;
    sizes_t *sizes;
};
#endif