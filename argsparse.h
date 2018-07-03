
/*
These three macros can handle 99% of argument parsing cases and 
in pretty much every comparison test it was >50% faster than
PyArg_UnpackTuple:

DEFINE_VARARGS(function name, min args, format codes...)
VARARG(type, variabl name, default value...) 
PARSE_VARARGS()

Seriously need a refactor so PARSE_VARARGS can handle variable return 
types in a platform independent way.
*/

#define VARARG(tp, name, value, ...) \
    tp name = value __VA_ARGS__;\
    vars[_VA_INDEX_++].any = &name
static int vararg_default_impl(void *arg, Py args, ptr_t *vars) {
    ssize_t n;
    for(n = 0; n<Py_SIZE(args); ++n){
        if(vars[n].any == arg)
            return 0;
    }
    return 1;
}
#define VA_USE_DEFAULT(arg) vararg_default_impl(&arg, args, vars)
#define PVARARG(arg, ...) \
    __VA_ARGS__ vars[_VA_INDEX_++].any = &arg
#define AP_VARARG(arg) \
    vars[_VA_INDEX_++].any = arg
#define DEFINE_VARARGS(func,  minargs, ...) \
    PyObject *ob_stack[8] = {NULL};\
    static argparser parsers[8] = {DEF_PARSERS(__VA_ARGS__)};\
    ptr_t vars[VA_LEN(__VA_ARGS__)];\
    int _VA_INDEX_ = 0;\
    static ArgSpec _argspec = {\
        .fname = #func,\
        .vnames = NULL,\
        .min_args = minargs,\
        .max_args = VA_LEN(__VA_ARGS__),\
        .allow_Nones = NULL,\
        .parsers=parsers\
    };\
    static const char *varnames[VA_LEN(__VA_ARGS__)]
#if defined(RAND_DEBUG)
#define NUM_NONES (sizeof(_Nones) / sizeof(_Nones[0]))
#define NUM_VARS (sizeof(vars) / sizeof(vars[0]))
#define VARARGS_NONES(...) \
    static const int _Nones[] = {__VA_ARGS__};\
    if(NUM_NONES != NUM_VARS)\
        return PySystemError("num fmt None flags doesn't match num vars");\
    _argspec.allow_Nones = _Nones
#else 
#define VARARGS_NONES(...) \
    static const int _Nones[] = {__VA_ARGS__};\
    _argspec.allow_Nones = _Nones
#endif
#define CLEAR_VARARG_STACK(stack, stacksize) \
    do {\
        for(ssize_t j=0; j<stacksize; ++j)\
            Py_XDECREF(stack[j]);\
    } while(0)  

#define PARSE_VARARGS(...) \
    _argspec.vnames = varnames;\
    if(!unpack_args(args, &_argspec, vars, ob_stack))\
        return NULL
        
#define PARSE_VARARGS2(failvalue) \
    _argspec.vnames = varnames;\
    if(!unpack_args(args, &_argspec, vars, ob_stack))\
        return failvalue 
        
#define PARSE_VARARGS3(loc) \
    _argspec.vnames = varnames;\
    if(!unpack_args(args, &_argspec, vars, ob_stack))\
        goto loc
#define PARSE_VARARGS_WITH_RESULT(v) \
     _argspec.vnames = varnames;\
     v = unpack_args(args, &_argspec, vars, ob_stack)
#define PARSE_VARARGS_AND_RETURN_CALL(f, ...) \
    _argspec.vnames = varnames;\
    if(!unpack_args(args, &_argspec, vars, ob_stack))\
        return f(__VA_ARGS__);
#define PARSE_VARARGS_FAST() \
    ((_argspec.vnames=varnames),unpack_args(args, &_argspec, vars, ob_stack))
#define VA_LEN(...) VA_PACK(VA_LPAD_1(__VA_ARGS__))
#define VA_PAD(...) VA_PACK2(VA_LPAD_1(__VA_ARGS__))
#define VA_LPAD_1(...) unused, __VA_ARGS__
#define XPAND1(x) x
#define VA_PACK(...) XPAND1(_VA_LEN_(__VA_ARGS__, 7, 6, 5, 4, 3, 2, 1, 0))
#define VA_PACK2(...) XPAND1(_VA_LEN_2(__VA_ARGS__, 0, 0, 0, 0, 0, 0, 0, 0))
#define _VA_LEN_(_0,_1,_2,_3,_4,_5,_6,_7, count, ...) count
#define _VA_LEN_2(u,_0,_1,_2,_3,_4,_5,_6,_7,...) _PARSERS(_0,_1,_2,_3,_4,_5,_6,_7)

#define DEF_PARSERS(...) VA_PAD(__VA_ARGS__)
#define _AP_CASE_(pred, tail, ifelse) (pred? arg_parse_##tail : ifelse)
#define _PARSERS(_0,_1,_2,_3,_4,_5,_6,_7) \
    GET_PARSER(_0),\
    GET_PARSER(_1),\
    GET_PARSER(_2),\
    GET_PARSER(_3),\
    GET_PARSER(_4),\
    GET_PARSER(_5),\
    GET_PARSER(_6),\
    GET_PARSER(_7)
#define PARSE_ERROR(name) \
    static int name(PyObject *args, ArgSpec *fmt)
#define c_int(x) (int)(x)
#define ERR(tp,...) return Py##tp##Error(__VA_ARGS__), 0;
#define PARSER_ARGS ArgSpec *fmt, PyObject *arg, ptr_t *vars, PyObject **stack
#define PARSER(code) static int arg_parse_##code (PARSER_ARGS)
static int int_parser(PyObject *arg, rand64_t *var, int fmt, int overflow) {
    py_alias_t integral;
    if(!(integral.ob=py_index(arg)))
        return FALSE;
    int result = py_int_parse(integral.int_, var, fmt, overflow);
    Py_DECREF(integral.ob);
    return result; 
}
PARSER(o) {
    // PyObject with no ref count increase
    *vars->o = arg;
    return TRUE;
}
PARSER(d) {
    // Convert an object supporting the number protocol to a C double
    return py_number_as_double(arg, vars->d);
}
PARSER(L) {
    // convert an object to a python int via its __int__ (nb_int)) slot
    *stack = py_int_from_number(arg);
    PyObject **var = vars->o;
    return ((*var=*stack)!=NULL);
}
PARSER(K) {
    // convert an object to a python int via its __index__ (nb_index) slot
    *stack = py_index(arg);
    PyObject **var = vars->o;
    return ((*var=*stack)!=NULL);
}
PARSER(i) {
    // convert a "number" to a C int64_t with overflow raising an error
    return int_parser(arg, vars->v, 'i', FALSE);
}
PARSER(I) {
    // convert a "number" to a C int64_t ignoring overflow errors
    return int_parser(arg, vars->v, 'I', TRUE);
}
PARSER(u) {
    // convert a "number" to a C uint64_t with overflow raising an error
    return int_parser(arg, vars->v, 'u', FALSE);
}
PARSER(U) {
    // convert a "number" to a C int64_t ignoring overflow errors
    return int_parser(arg, vars->v, 'U', TRUE);
}
PARSER(n) {
    // convert a "number" to a C ssize_t with overflow raising an error
    py_alias_t integral;
    if(!(integral.ob=py_index(arg)))
        return FALSE;
    int result = py_int_as_sizes_t(integral.int_, vars->sizes, 'n');
    Py_DECREF(integral.ob);
    return result;
}
PARSER(N) {
    // convert a "number" to a C size_t with overflow raising an error
    py_alias_t integral;
    if(!(integral.ob=py_index(arg)))
        return FALSE;
    int result = py_int_as_sizes_t(integral.int_, vars->sizes, 'N');
    Py_DECREF(integral.ob);
    return result;    
}
PARSER(T) {
    int *i = vars->i;
    *i = PyObject_IsTrue(arg);
    return TRUE;
}

/*
Convert sequences (s) or iterables (S) to the PySequence abstraction

The first thing py_sequence_parse does when it detects the argument has
sequence methods is attach a borrowed reference to the PySequence.

If parsing succeeds, whatever winds up in the ->sq slot is a new reference 
that must have its reference count decreased if the parsing of any
proceeding arguments fails.
*/

PARSER(s) {
    // validate object is a sequence and convert to the PySequence abstraction

    int result = py_sequence_parse(arg, vars->sq, FALSE);
    if(result)
        *stack = pyseq_SQ(vars->sq);
    return result;
}
PARSER(S) {
    // convert any iterable into the PySequence abstraction
    int result = py_sequence_parse(arg, vars->sq, 1);
    if(result)
        *stack = pyseq_SQ(vars->sq);
    return result;
}
PARSER(null) {
    return PySystemError("if you ever see this error, buy a lottery "
                         "ticket (throwing this error may be better than "
                         "an interpreter crash, but probably isn't)"), 0;
}
FAST_FUNC(int)invalid_parser_spec (PARSER_ARGS) {
    return PySystemError("argument parser for the '%s' function has an "
                         "invalid format spec", fmt->fname),0;
}
PARSE_ERROR(null_arguments) {
    if(PyErr_Occurred())
        return 0;
    ERR(System, "unpack_args: received NULL args tuple");   
}
PARSE_ERROR(bad_arguments) {
    ERR(System, "unpack_args expected to unpack a tuple coming "
    "from the function %s; got a %s", fmt->fname, Py_TP_NAME(args));
}
PARSE_ERROR(invalid_num_args) {
    static const char *amod[] = {"s", ""};
    int const nmin = c_int(fmt->min_args);
    int const nmax = c_int(fmt->max_args);
    int const size = c_int(Py_SIZE(args));
    const char *fname = fmt->fname;
    
    if(!nmin)
        ERR(Type, "%s() takes no arguments", fname);    
    if(size>nmax)
        ERR(Type,"%s() takes %i positional argument%s but %i were given",
            fname, nmin, amod[nmin==1], size);
    char **vnames = ((char **)fmt->vnames) + size;
    
    if(size==(nmin-1)) 
        ERR(Type, "%s() missing required positional argument: '%s'",
            fname, *vnames);
    
    if(size==(nmin-2))
        ERR(Type, "%s() missing 2 required positional arguments: "
            "'%s' and '%s'", fname, *vnames, *(vnames+1));
    ERR(Type, "%s() missing %i required positional arguments",
        fname, nmin-size);
}    
FUNC(int) unpack_nones(Py args, ArgSpec *fmt, ptr_t *vars, Py *stack ){
    for(ssize_t i=0; i<Py_SIZE(args); ++i) {
        if(fmt->allow_Nones[i] && TUPLE_ITEMS(args)[i]==Py_None) 
            continue;
        if(!fmt->parsers[i](fmt, TUPLE_ITEMS(args)[i], vars + i, stack + i))
            return 0;
    }
    return 1;
}
FUNC(int) unpack_args(Py args, ArgSpec *fmt, ptr_t *vars, Py *stack) {

    if(!args)
        return null_arguments(args, fmt);
    if(!PyTuple_CheckExact(args))
        return bad_arguments(args, fmt);  
    if(Py_SIZE(args)==0 && fmt->min_args==0)
        return 1;
    if(fmt->min_args > Py_SIZE(args) || Py_SIZE(args) > fmt->max_args)
        return invalid_num_args(args, fmt);

    if(fmt->allow_Nones) {
        if(!unpack_nones(args, fmt, vars, stack))
            goto FAIL;
        return 1;
    }
    for(ssize_t i=0; i<Py_SIZE(args); ++i) {
        if(!fmt->parsers[i](fmt, TUPLE_ITEMS(args)[i], vars + i, stack + i))
            goto FAIL;
    }
    return 1;
    
  FAIL:
    CLEAR_VARARG_STACK(stack, Py_SIZE(args));
    return 0;
}
#ifdef DEBUG_PARSE_ARGS 
#define STRKITTY2(co) debug_##co
#define STRKITTY(co) STRKITTY1(debug_##co)
#define STRKITTY1(x) #x

#define DEBUG_METHOD(co)\
{STRKITTY(co), (PyCFunction)STRKITTY2(co), METH_VARARGS, NULL}
#define DEBUG_FUNC(co, co_num, var_tp, var_default, rspec, rcast) \
static PyObject *debug_##co(PyObject *self, PyObject *args) {\
    DEFINE_VARARGS(debug##co, 1, co_num) = {#co};\
    VARARG(var_tp, co, var_default);\
    PARSE_VARARGS();\
    return pbv(rspec, (rcast)co);\
    }

static PyObject *debug_rand64_t(PyObject *self, PyObject *args ) {
    DEFINE_VARARGS("debug_rand64_t", 4, 'i', 'I', 'u', 'U')={"i","I","u","U"};
    VARARG(rand64_t, i, {0});
    VARARG(rand64_t, I, {0});
    VARARG(rand64_t, u, {0});
    VARARG(rand64_t, U, {0});
    PARSE_VARARGS();
    return pbv("LLKK", i.i, I.i, u.u, U.u);
}
DEBUG_FUNC(o, 'o', PyObject*, NULL, "O", PyObject*)
DEBUG_FUNC(d, 'd', double, 0.0, "d", double)
DEBUG_FUNC(L, 'L', PyObject*, NULL, "N", PyObject*)
DEBUG_FUNC(K, 'K', PyObject*, NULL, "N", PyObject*)
DEBUG_FUNC(n, 'n', ssize_t, 0, "L", int64_t)
DEBUG_FUNC(N, 'N', size_t, 0, "K", uint64_t)
DEBUG_FUNC(T, 'T', int, -1, "T", int)
#endif
