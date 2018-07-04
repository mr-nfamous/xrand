static const char module_doc[] = (
"\
    xrand - e(x)tremely fast general use PRNG library for CPython\n\
\n\
    Implemented entirely in C with micro-optimized code, including\n\
    a significant rewrite of Python's abstract object API, xrand gives\n\
    Python the ability to compete with compiled languages for developing\n\
    software that makes heavy use of random number generation.\n\
\n\
    The underlying generator will be either xorshift128+ or xoroshiro128+.\n\
    By default, 32 bit platforms use xorshift while 64 bit use xoroshiro.\n\
    Since the way they are implemented makes speed the only difference\n\
    between the two, which one is faster is which one is used.\n\
\n\
    Except the SystemRandom class (since these generators aren't CS-PRNGS),\n\
    xrand has everything the standard library module random.py has and more.\n\
    The three new core functions are `flip`, `rand_bytes`, and `shuffled`,\n\
    where `flip` is designed to make 50/50 choices super fast, `rand_bytes`\n\
    can write its output into a buffer, and `shuffled` efficiently creates\n\
    a new, randomized list based on any iterable.\n\
\n\
    Almost every xrand function is at least one order of magnitude faster\n\
    to call than the standard lib version, with several approaching two.\n\
    Besides faster functions calls, xrand also supports parallel substreams\n\
    guaranteed to non-correlated, which opens up the possibility of using\n\
    multiprocessing that doesn't call into question the data's validity.\n\
\n\
    Most of the new functionality is in the form of a set of new objects\n\
    that work conceptually like functools.partial. They are built by passing\n\
    the arguments its respective implementation needs to a special\n\
    constructor method that builds the object and then instead of calling\n\
    the implementation function right away, calls the argument parser that\n\
    method uses and keeps the result stored in memory for later use.\n\
\n\
    That's the new. Here's the different:\n\
\n\
    LIST OF AFOREMENTIONED OBJECT CONSTRUCTORS:\n\
\n\
    * choices: returns a `choices` object used with both select and sample\n\
    * dice: returns a `dice` object complementing rand_int\n\
\n\
    All of the following methods return an iterator object: `randiter`\n\
\n\
    * iter_beta\n\
    * iter_expo\n\
    * iter_gamma\n\
    * iter_lognorm\n\
    * iter_norm\n\
    * iter_pareto\n\
    * iter_triangular\n\
    * iter_uniform\n\
    * iter_vonmises\n\
    * iter_weibull\n\
    * flips (not really comparable to `flip`, actually...)\n\
\n\
    MORE ON THE CORE FUNTIONS NOT IN random.py:\n\
\n\
    Speaking of `flip`, while it might seem silly or pointless, but when\n\
    a simple 50/50 binary choices is needed, nothing else going to come\n\
    close to its efficiency.\n\
\n\
    Second up is `rand_bytes` which is used to fill some object supporting\n\
    the buffer protocol with random bytes. When given nothing to fill\n\
    it simply returns a new bytes object. Currently it is pretty limited,\n\
    but it does work with array.array and possibly some types of np.array.\n\
\n\
    The final addition is `shuffled`. The difference between it and\n\
    `shuffle` is analogous to the difference between the builtin `sorted()`\n\
    function and `list.sort`. Like `sorted()`, `shuffled` first unpacks\n\
    any iterable into a brand new list but rather than sorting the new list,\n\
    it shuffles it. Adding a second way to shuffle allows `shuffle`\n\
    to limit itself to work only Python lists, which allows it to remain\n\
    around 2 orders of magnitude faster than the shuffle from random.py.\n\
\n\
    The only functionality xrand doesn't explicitly implement compared\n\
    to random.py is the case of using `random.randrange` with multiple\n\
    arguments of very large numbers. xrand.select on a range object very\n\
    efficiently takes care the multiple argument case when reasonable sized\n\
    ints are used, and the new, highly optimized `rand_index` can and should\n\
    replace the one argument case.\n\
\n\
    xrand.choices/select/sample is so radically different from than std lib\n\
    version it might as well be considered a new addition as well.\n\
    Rather than using THREE separate *sequences* for weighted `choice`s,\n\
    A single dict can be used with the keys as members and values as weights.\n\
\n\
    If for whatever reason (what reason???) the population *must* contain\n\
    unhashhables members, don't worry. The `Population` object is a mapping\n\
    type that accepts unhashable keys by wrapping them with a `PopEntry` to\n\
    guarantee it can be both inserted and retrieved from any Python dict\n\
    or set. Also, sampling actually does work with weights.\n\
\n\
    STUFF WITH DIFFERENT NAMES:\n\
\n\
    xrand was started with the goal of becoming a drop in replacement for\n\
    the ancient random.py, perhaps even one day becoming part of the stdlib.\n\
    However as it written before pep8, the names don't cut it for 2018.\n\
\n\
    In the below table, the call signatures of the random.py functions\n\
    are compatible with the xtra version, but not vice versa. For example,\n\
    random.randint(a,b) versus xrandom.randint(a, b=0). `choices`, which\n\
    is technically not at all the same thing is the sole exception since\n\
    it doesn't immediately return a result.\n\
\n\
\n\
    random.py       : xrand\n\
    __reduce__      : __getnewargs_ex__\n\
    --------------- : __copy__\n\
    _randbelow      : rand_index (only difference is it is capped to sys SIZE_MAX)\n\
    betavariate     : rv_beta\n\
    choice          : select\n\
    choices         : select\n\
    --------------- : choices (constructor for select/sample complement)\n\
    --------------- : dice\n\
    expovariate     : rv_expo\n\
    --------------- : flip\n\
    --------------- : flips\n\
    gammavariate    : rv_gamma\n\
    gauss           : rv_normal (normalvariate and gauss do the same thing)\n\
    getrandits      : rand_bits\n\
    getstate        : state / pstate (read only data descriptors)\n\
    --------------- : iter_beta\n\
    --------------- : iter_expo\n\
    --------------- : iter_gamma\n\
    --------------- : iter_log_normal\n\
    --------------- : iter_normal\n\
    --------------- : iter_pareto\n\
    --------------- : iter_triangular\n\
    --------------- : iter_uniform\n\
    --------------- : iter_vonmises\n\
    --------------- : iter_weibull\n\
    --------------- : jump\n\
    logormvariate   : rv_log_normal\n\
    normalvariate   : rv_normal\n\
    paretovariate   : rv_pareto\n\
    --------------- : rand_bytes\n\
    randint         : rand_int\n\
    randrange       : ---------------\n\
    random          : rand_float\n\
    sample          : sample\n\
    --------------- : split\n\
    triangular      : rv_triangular\n\
    uniform         : rv_uniform\n\
    vonmisesvariate : rv_vonmises\n\
    weibullvariate  : rv_weibull\n\
\n\
\n\
    NOTES ON THE UNDERLYING GENERATORS SEED/STATE:\n\
\n\
    The underlying generator state is structured like this:\n\
\n\
    * uint64_t state[2]\n\
    * uint64_t seed[2]\n\
    * size_t pstate\n\
    * uint8_t rb\n\
\n\
    Seed:\n\
\n\
    The generator's state can optionally be initialized to `seed`. When\n\
    provided, the seed must either be an integer or a buffer protocol\n\
    supporting object, such as bytes, memoryview, or an array.array.\n\
\n\
    When an integer seed is provided it must not not be zero or require more\n\
    than 128 bits to represent. For buffers, only up to 16 bytes will be\n\
    read, and those first 16 bytes similarly must not be all-zero.\n\
\n\
    Since the underlying generator recovers from a bad seed in less than 8\n\
    iterations, (unlike MT19937, which can take 100s of thousands) there's\n\
    no reason to modify the input. Since there's only at most 128 bytes to\n\
    contend with, reproducibility can be much easier.\n\
\n\
    When a seed isn't provided, a suitable one is automatically generated\n\
    and each interpreter process is guaranteed to never generate the same\n\
    seed twice.\n\
\n\
    State:\n\
\n\
    Besides floats, which consume 52 of the 60 bits the internal generator\n\
    outputs each time it is called, most functions rarely need more than\n\
    16, so some of the functions share a small pool of random bits.\n\
    Throwing away so many bits doesn't really matter on 64 bit platforms,\n\
    but since it is a lot more expensive to generate the next random number\n\
    on a 32 bit platforms while costing practically nothing to keep up\n\
    with the pool on 64 bit, it is worth the effort..\n\
\n\
    The main state, which is composed of 2 64 bit unsigned integers, is\n\
    viewed with the `state` descriptor.\n\
\n\
    And now we're at the end, at last:\n\
\n\
    MISCELLANIOUS STUFF:"
);
#define bint enum _bint
#define py_bytes PyBytesObject
#define py_dict PyDictObject
#define py_float PyFloatObject
#define py_int PyLongObject
#define py_list PyListObject
#define py_set PySetObject
#define py_tuple PyTupleObject
#define py_type PyTypeObject
#define py_unicode PyUnicodeObject
#define GET_PARSER(v) \
( !v   ? NULL:\
(v=='o'? arg_parse_o:\
(v=='d'? arg_parse_d:\
(v=='L'? arg_parse_L:\
(v=='K'? arg_parse_K:\
(v=='i'? arg_parse_i:\
(v=='I'? arg_parse_I:\
(v=='u'? arg_parse_u:\
(v=='U'? arg_parse_U:\
(v=='n'? arg_parse_n:\
(v=='N'? arg_parse_N:\
(v=='T'? arg_parse_T:\
(v=='s'? arg_parse_s:\
(v=='S'? arg_parse_S:\
invalid_parser_spec))))))))))))))
PyObject *Random___copy__(RandomObject *rng);
static char Random___copy___doc[] = (
"\
    __copy__()\n\
    Get a new copy of `self`"
);
PyObject *Random___getnewargs_ex__(RandomObject *rng);
static char Random___getnewargs_ex___doc[] = (
"\
    __getnewargs_ex__()\n\
    Helper for pickle\n\
\n\
    The constructor for Random takes 1 or 0 arguments. If it gets an\n\
    argument that is a tuple it assumes it's supposed to unpickle it.\n\
    There's no reason for this output to be human readable but it might\n\
    as well be documented here what the garbage __getnewargs_ex__\n\
    returns represents...\n\
\n\
    >>> s0, s1 = rng.state\n\
    >>> seed = rng.seed\n\
    >>> p_val, p_bits = rng.pstate\n\
\n\
    >>> import struct\n\
    >>> packed = struct.pack('QQ16sNN', s0, s1, seed, p_val, p_bits)\n\
    >>> pickled = struct.unpack('32sNN', packed)\n\
    >>> new = Random(pickled).state\n\
    >>> new.state == rng.state and new.pstate == rng.pstate\n\
    True\n\
    >>> rng.flip('h')\n\
    False\n\
    >>> new.state == rng.state and new.pstate == rng.pstate\n\
    False"
);
PyObject *Random_choices(RandomObject *rng, PyObject *arg);
static char Random_choices_doc[] = (
"\
    choices(population) -> choices\n\
    Get a new 'choices' object based on `population`\n\
\n\
    On every function call before it can actually make a `choice`,\n\
    `Random.choice` must first convert the abstract Python object form\n\
    `population` is stored as into a machine-readable representation.\n\
\n\
    This process of converting Python objects to more fundamental data\n\
    types is oftentimes the most expensive segment of a function call.\n\
    Thus, keeping the arguments in their most fundamental types can\n\
    significantly increase performance if multiple function calls\n\
    taking the same arguments is needed, especially if all iteration\n\
    is done at the C level.\n\
\n\
    `choices` objects have the following methods:\n\
\n\
    * next():\n\
    Identical to:\n\
    >>> f = partial(rng.select, population)\n\
    >>> f()\n\
\n\
    * next_n(n):\n\
    Identical to:\n\
    >>> f = partial(rng.select, population)\n\
    >>> [f() for i in range(n)]\n\
\n\
    * sample(k):\n\
    Identical to:\n\
    >>> f= partial(rng.sample, population)\n\
    >>> f(k)\n\
    Note: only between 5-10% faster than sample. Including\n\
    `choices` construction time, takes many calls to reach\n\
    parity with just using `sample`\n\
\n\
    along with the following data descriptors:\n\
\n\
    * ro.population:\n\
    The population being stored. For dict based `choices`\n\
    objects it returns two lists; the first are all unique\n\
    elements. The other is a list equivalent to\n\
    accumulate(p.values()).\n\
\n\
    For ranges, strings, tuples, and lists, the original\n\
    sequence is returned. Everything else will have been\n\
    converted to a  list.\n\
\n\
    * ro.size:\n\
    Population size. For dicts, this is the effective count\n\
    of  the members in the population (sum(pop.values())).\n\
    For everything else it is simply the length of whatever\n\
    sequence representation `choices`  decided to convert it to.\n\
\n\
    The caveats for `Random.select` and `Random.sample` still apply.\n\
\n\
    Here's some Python level benchmarking between this and random.py:\n\
    (number of random.choice(pop) equivalent calls per second)\n\
\n\
    >>> from stex import e\n\
    >>> from random import choice as pychoice, sample as pysample\n\
    >>> from xrand import select, choices, sample\n\
    >>> pop = range(123456)\n\
    >>> r = e('pychoice(pop)'); print(f'{r:_.0f}')\n\
    654_141\n\
    >>> r = e('select(pop)'); print(f'{r:_.0f}')\n\
    8_392_386\n\
    >>> f = choices(pop).next; r = e('f()'); print(f'{r:_.0f}')\n\
    11_400_663 \n\
    >>> r = e('[pychoice(pop) for i in range(100)]')*100; print(f'{r:_.0f}')\n\
    644_486\n\
    >>> r = e('[select(pop) for i in range(100)]')*100; print(f'{r:_.0f}')\n\
    6_920_097\n\
    >>> f = choices(pop).next_n; r = e('f(100)')*100; print(f'{r:_.0f}')\n\
    22_869_879\n\
\n\
    >>> cards = itertools.product('HSDC',(*'23456789','10',*'JKQA'))\n\
    >>> cards = [f'{a}{b}' for a,b in cards]\n\
    >>> r = e('pysample(cards, 20)'); print(f'{r:_.0f}')\n\
    29_768\n\
    >>> r = e('sample(cards, 20)'); print(f'{r:_.0f}')\n\
    1_165_779\n\
\n\
    `select(pop)`  -> 13x faster\n\
    `.next()       -> 17x faster\n\
    `.next_n(100)` -> 35x faster\n\
    `sample(cards) -> 39x faster (to be fair, varies 20-50x)"
);
PyObject *Random_dice(RandomObject *rng, PyObject *args);
static char Random_dice_doc[] = (
"\
    dice(a=0, b=0) -> dice\n\
    Get a new `dice' object that rolls random ints from [a, b]\n\
\n\
    This object has 2 methods:\n\
\n\
    * next():\n\
    Identical to:\n\
    >>> f = partial(rng.rand_int, a, b)\n\
    >>> f()\n\
\n\
    * next_n(n):\n\
    Identical to:\n\
    >>> f = partial(rng.select, a, b)\n\
    >>> [f() for i in range(n)]\n\
\n\
    along with the following data descriptors:\n\
\n\
    * ro.max:\n\
    Highest roll of the dice\n\
\n\
    * ro.min:\n\
    Lowest roll of the dice"
);
PyObject *Random_flip(RandomObject *rng, PyObject *arg);
static char Random_flip_doc[] = (
"\
    flip(heads_value) -> object\n\
    50% chance to return either `heads_value` or False"
);
PyObject *Random_flips(RandomObject *rng, PyObject *args);
static char Random_flips_doc[] = (
"\
    flips(n, heads_odds=0.5, heads=True, tails=False)\n\
    Iterator producing `n` binary choices\n\
\n\
    Every loop has a `heads_odds` chance to return `heads which\n\
    means it has a `(1.0 - heads_odds)` chance to return `tails.\n\
\n\
    If `n` is None the iterator will repeat forever.\n\
\n\
    The iterator's `take` method can call `__next__` without\n\
    reducing the number of remaining iterations and it can be used\n\
    even after the iterator has been exhausted."
);
PyObject *Random_iter_beta(RandomObject *rng, PyObject *args);
static char Random_iter_beta_doc[] = (
"\
    iter_beta(n, alpha, beta) -> randiter\n\
    Iterate over` n` random variables with beta distribution\n\
    If `n` is None the iterator will repeat forever.\n\
\n\
    The iterator's `take` method can call `__next__` without\n\
    reducing the number of remaining iterations and it can be used\n\
    even after the iterator has been exhausted."
);
PyObject *Random_iter_expo(RandomObject *rng, PyObject *args);
static char Random_iter_expo_doc[] = (
"\
    iter_expo(n, lamda) -> randiter\n\
    Iterate over` n` random variables with exponential distribution\n\
    If `n` is None the iterator will repeat forever.\n\
\n\
    The iterator's `take` method can call `__next__` without\n\
    reducing the number of remaining iterations and it can be used\n\
    even after the iterator has been exhausted."
);
PyObject *Random_iter_gamma(RandomObject *rng, PyObject *args);
static char Random_iter_gamma_doc[] = (
"\
    iter_gamma(n, alpha, beta) -> randiter\n\
    Iterate over` n` random variables with gamma distribution\n\
    If `n` is None the iterator will repeat forever.\n\
\n\
    The iterator's `take` method can call `__next__` without\n\
    reducing the number of remaining iterations and it can be used\n\
    even after the iterator has been exhausted."
);
PyObject *Random_iter_log_normal(RandomObject *rng, PyObject *args);
static char Random_iter_log_normal_doc[] = (
"\
    iter_log_normal(n, mu, sigma) -> randiter\n\
    Iterate over` n` log-normally distributed random variables\n\
    If `n` is None the iterator will repeat forever.\n\
\n\
    The iterator's `take` method can call `__next__` without\n\
    reducing the number of remaining iterations and it can be used\n\
    even after the iterator has been exhausted."
);
PyObject *Random_iter_normal(RandomObject *rng, PyObject *args);
static char Random_iter_normal_doc[] = (
"\
    iter_normal(n, mu, sigma) -> randiter\n\
    Iterate over` n` normally distributed random variables\n\
    If `n` is None the iterator will repeat forever.\n\
\n\
    The iterator's `take` method can call `__next__` without\n\
    reducing the number of remaining iterations and it can be used\n\
    even after the iterator has been exhausted."
);
PyObject *Random_iter_pareto(RandomObject *rng, PyObject *args);
static char Random_iter_pareto_doc[] = (
"\
    iter_pareto(n, alpha) -> randiter\n\
    Iterate over` n` random variables with Pareto distribution\n\
    If `n` is None the iterator will repeat forever.\n\
\n\
    The iterator's `take` method can call `__next__` without\n\
    reducing the number of remaining iterations and it can be used\n\
    even after the iterator has been exhausted."
);
PyObject *Random_iter_triangular(RandomObject *rng, PyObject *args);
static char Random_iter_triangular_doc[] = (
"\
    iter_triangular(n, lo, hi, c) -> randiter\n\
    Iterate over` n` random variables with triangular distribution\n\
    If `n` is None the iterator will repeat forever.\n\
\n\
    The iterator's `take` method can call `__next__` without\n\
    reducing the number of remaining iterations and it can be used\n\
    even after the iterator has been exhausted."
);
PyObject *Random_iter_uniform(RandomObject *rng, PyObject *args);
static char Random_iter_uniform_doc[] = (
"\
    iter_uniform(n, a, b) -> float\n\
    Iterate over `n` random variables from the uniform distribution [a,b)\n\
    If `n` is None the iterator will repeat forever.\n\
\n\
    The iterator's `take` method can call `__next__` without\n\
    reducing the number of remaining iterations and it can be used\n\
    even after the iterator has been exhausted."
);
PyObject *Random_iter_vonmises(RandomObject *rng, PyObject *args);
static char Random_iter_vonmises_doc[] = (
"\
    iter_vonmises(n, mu, kappa) -> float\n\
    Iterate over` n` random variables with Vonmises distribution\n\
    If `n` is None the iterator will repeat forever.\n\
\n\
    The iterator's `take` method can call `__next__` without\n\
    reducing the number of remaining iterations and it can be used\n\
    even after the iterator has been exhausted."
);
PyObject *Random_iter_weibull(RandomObject *rng, PyObject *args);
static char Random_iter_weibull_doc[] = (
"\
    iter_weibull(n, alpha, beta) -> float\n\
    Iterate over` n` random variables with Weibull distribution\n\
    If `n` is None the iterator will repeat forever.\n\
\n\
    The iterator's `take` method can call `__next__` without\n\
    reducing the number of remaining iterations and it can be used\n\
    even after the iterator has been exhausted."
);
PyObject *Random_jump(RandomObject *rng);
static char Random_jump_doc[] = (
"\
    jump() -> None\n\
    Advances the underlying generator state by 2**64 iterations"
);
PyObject *Random_rand_bits(RandomObject *rng, PyObject *arg);
static char Random_rand_bits_doc[] = (
"\
    rand_bits(bits) -> int\n\
    Random int from uniformly-distributed range [0, 2**bits)"
);
PyObject *Random_rand_bytes(RandomObject *rng, PyObject *args);
static char Random_rand_bytes_doc[] = (
"\
    rand_bytes(size, buf=None) -> object\n\
    Generates `size` random bytes\n\
\n\
    With only the `size` argument, return a new bytes object filled\n\
    with `size` random bytes. If the optional `buf` argument is\n\
    supplied, it must be an object supporting the buffer protocol.\n\
    A type error will be raised if the buffer is not C-style contiguous\n\
    or is read-only.    \n\
\n\
    Examples:\n\
    >>> rand_bytes(5)\n\
    b'abcde'\n\
\n\
    >>> x = array.array('Q', [0])\n\
    >>> rand_bytes(8, x)\n\
    array([10583363146662955482], 'Q')\n\
\n\
    >>> import numpy as np\n\
    >>> x = rand_bytes(3*4, np.zeros(3, 'i'))\n\
    >>> y = rand_bytes(len(x)*4, x)\n\
    >>> x is y, y # note the original object is modified...\n\
    (True, array([-1338281059, -1828093642,  1742631451], dtype=int32))"
);
PyObject *Random_rand_float(RandomObject *rng);
static char Random_rand_float_doc[] = (
"\
    rand_float() -> float\n\
    Random float from uniformly distributed half-open interval [0, 1.0)"
);
PyObject *Random_rand_index(RandomObject *rng, PyObject *arg);
static char Random_rand_index_doc[] = (
"\
    rand_index(i) -> int\n\
    Random int from the uniformly distributed range [0, i)\n\
\n\
    `i` must be positive and able to fit in a platform size_t.\n\
    The exact maximum value may be seen from within python with:\n\
\n\
    >>> from ctypes import *; 2**(sizeof(c_size_t) * 8)-1\n\
\n\
    In general this is 2**64-1 for 64 and 2**32 for 32 bit platforms.\n\
\n\
    Because the result is from the half-open interval [0, i) and\n\
    since 2**bits-1 is the maximum value `i` can be without an overflow\n\
    error, 2**bits-1 can never be a return value from this function."
);
PyObject *Random_rand_int(RandomObject *rng, PyObject *args);
static char Random_rand_int_doc[] = (
"\
    rand_int(a, b=None) -> int\n\
    Random int from the uniformly-distributed range [a, b]\n\
\n\
    As with `rv_uniform`, the order of `a` and `b` is of no consequence:\n\
\n\
    All four of the following function calls are identical:\n\
\n\
    >>> rand_int(-10)\n\
    >>> rand_int(0, -10)\n\
    >>> rand_int(-10, 0)\n\
    >>> select(range(-10, 1)\n\
\n\
    Keep in mind that just the presence of a second argument causes\n\
    an extra 20% overhead to come out of nowhere, even if it's as\n\
    simple as the difference between rand_int(0, 20) and rand_int(20)\n\
\n\
    Also see `Random.dice`"
);
PyObject *Random_rv_beta(RandomObject *rng, PyObject *args);
static char Random_rv_beta_doc[] = (
"\
    rv_beta(alpha, beta) -> float\n\
    Random variable with beta distribution"
);
PyObject *Random_rv_expo(RandomObject *rng, PyObject *arg);
static char Random_rv_expo_doc[] = (
"\
    rv_expo(lamda) -> float\n\
    Random variable with exponential distribution"
);
PyObject *Random_rv_gamma(RandomObject *rng, PyObject *args);
static char Random_rv_gamma_doc[] = (
"\
    rv_gamma(alpha, beta) -> float\n\
    Random variable with gamma distribution"
);
PyObject *Random_rv_log_normal(RandomObject *rng, PyObject *args);
static char Random_rv_log_normal_doc[] = (
"\
    rv_log_normal(mu, sigma) -> float\n\
    Random variable with log normal distribution"
);
PyObject *Random_rv_normal(RandomObject *rng, PyObject *args);
static char Random_rv_normal_doc[] = (
"\
    rv_normal(mu, sigma) -> float\n\
    Random variable with normal (Gaussian) distribution"
);
PyObject *Random_rv_pareto(RandomObject *rng, PyObject *arg);
static char Random_rv_pareto_doc[] = (
"\
    rv_pareto(alpha) -> float\n\
    Random variable with Pareto distribution"
);
PyObject *Random_rv_triangular(RandomObject *rng, PyObject *args);
static char Random_rv_triangular_doc[] = (
"\
    rv_triangular(low, high, mode) -> float\n\
    Random variable with triangular distribution"
);
PyObject *Random_rv_uniform(RandomObject *rng, PyObject *args);
static char Random_rv_uniform_doc[] = (
"\
    rv_uniform(a, b) -> float\n\
    Random float from a uniform distribution [a, b)"
);
PyObject *Random_rv_vonmises(RandomObject *rng, PyObject *args);
static char Random_rv_vonmises_doc[] = (
"\
    rv_vonmises(mu, kappa) -> float\n\
    Random variable with Vonmises distribution"
);
PyObject *Random_rv_weibull(RandomObject *rng, PyObject *args);
static char Random_rv_weibull_doc[] = (
"\
    rv_weibull(alpha, beta) -> float\n\
    Random variable with Weibull distribution"
);
PyObject *Random_sample(RandomObject *rng, PyObject *args);
static char Random_sample_doc[] = (
"\
    sample(pop, k) -> list\n\
    Choose `k` elements from the `pop`ulation without replacement\n\
\n\
\n\
    If `pop` is a dictionary, the dict keys represent the members\n\
    of the populations and the corresponding values represent the\n\
    number of members in the population. Unlike `select` which allows\n\
    both int and float-based weighting, `sample` only acceptes ints.\n\
\n\
    When `pop` is a sequence or iterable, each *index* can only be\n\
    chosen once, which means that duplicates just increase the odds\n\
    that member will be chosen.\n\
\n\
\n\
    \"Replacement\" means that if the population is \"abcde\" for example,\n\
    once \"a\" is chosen it can never be chosen again. Internally, since\n\
    what is actually chosen is the element's index, what it really\n\
    means it that the number `0` out of the total population size (5)\n\
    will never be chosen again. This also applies when weightings\n\
    are provided. {\"a\": 4, \"b\":1} is logically converted to the list\n\
    [\"a\", \"a\", \"a\", \"a\", \"b\"] so that up to 4 \"a\" can be selected.\n\
\n\
    Example:\n\
\n\
    >>> sample(range(52))\n\
    [44, 18, 4, 13, 33]\n\
    >>> pop = Counter(\"aaabbbccc\")\n\
    >>> pop\n\
    Counter({\"a\":3, \"b\":3, \"c\":3})\n\
    >>> sample(pop, 6)\n\
    ['c', 'a', 'c', 'c', 'b', 'a']\n\
\n\
    The Counter example is functionally identical to:\n\
\n\
    >>> random.sample(list(pop.elements()), 5)\n\
\n\
    While sampling from a tuple or list will perform about 50% faster,\n\
    in the right circumstances (a large population with few uniques),\n\
    the dict based form will use orders of magnitude less memory.\n\
\n\
    >>> pop = ''.join(choices(string.ascii_lowercase).next_n(100000))\n\
    >>> popc= Counter(pop)\n\
\n\
    `pop` in the above example requires 400-800KB of memory whereas\n\
    `popc` only requires about 1KB.\n\
\n\
    See the documentation for `choices` and `select` for more info,\n\
    including how to handle the corner case of a weighted population\n\
    that has unhashable members."
);
PyObject *Random_select(RandomObject *rng, PyObject *arg);
static char Random_select_doc[] = (
"\
    select(population) -> object\n\
    Select a random member of `population`\n\
\n\
    `population` may be any iterable or dict/xrand.Population instance.\n\
\n\
    If `population` is an iterator/sequence, each member of the\n\
    population has a uniform (1/length) chance to be selected.\n\
\n\
    If `population` is a dict or `xrand.Population` instance, the\n\
    \"keys\" represent the population population members and the \"values\"\n\
    represent the weighting given to that particular member.\n\
\n\
    Member weightings may be represented as floats or integers.\n\
    However, all weightings for any given population must have be of\n\
    the same type.\n\
\n\
    This dict based population is invalid since it mixes ints and floats:\n\
\n\
    >>> {\"a\":1.0, \"b\":2.0, \"c\":1}\n\
\n\
    The keys in both of the following dicts will end up having roughly\n\
    the same odds of chosen:\n\
\n\
    >>> i_based = {\"a\":  1, \"b\":   2, \"c\":   7}\n\
    >>> f_based = {\"a\":0.5, \"b\": 1.0, \"c\": 3.5}\n\
\n\
    # Only \"roughly\" the same odds because floats are inherently\n\
    # imprecise (although insignificantly so, in this case)\n\
\n\
    - \"a\" will have a 1/10 (0.5/5.0) chance of being selected\n\
    - \"b\" will have a 2/10 (1.0/5.0) chance of being selected\n\
    - \"c\" will have a 7/10 (3.5/5.0) chance of being selected\n\
\n\
    `Random.choices` objects are special population members.\n\
    If the member chosen happens to be a `choices` instance\n\
    its `next` method is automatically (and recursively) invoked.\n\
\n\
    This could be used to implement and NPC drop table on an RPG\n\
    which could look something like this:\n\
\n\
    >>> coins_table = choices(range(200, 450))\n\
    >>> SCARY_PIRATE_DROPTABLE = {\n\
    None: 16,\n\
    coins_table: 12,\n\
    choices({\n\
    \"Health potion\":15,\n\
    \"Elixir of life\":1\n\
    }): 3,\n\
    choices({\n\
    'Eyepatch':6,\n\
    'Deadly cutlass':1,\n\
    choices({\n\
    'Treasure box':15,\n\
    'Meteor ring':10,\n\
    'Ancient coin':5,\n\
    'Rusty hook':1,\n\
    'Bloody bandana':1\n\
    }):1\n\
    }): 1}\n\
\n\
    >>> npc.get_loot = choices(SCARY_PIRATE_DROPTABLE).next\n\
\n\
    `npc.get_loot()` will return items with the following drop rates:\n\
\n\
    #                       roll #1   roll #2   roll #3    effective\n\
    * None:             ( 16/32)                     = [4096/8192]\n\
    * 200-450 coins:    ( 12/32)                     = [3072/8192]\n\
    * \"Health potion\":  (( 3/32) * (15/16))          = [ 720/8192]\n\
    * \"Elixir of life\": (( 3/32) * ( 1/16))          = [  48/8192]\n\
    * \"Eyepatch\":       (( 1/32) * ( 6/8))           = [ 192/8192]\n\
    * \"Deadly cutlass\": (( 1/32) * ( 1/8))           = [  32/8192]\n\
    * \"Treasure box\":   (( 1/32) * ( 1/8)) * (15/32) = [  15/8192]\n\
    * \"Meteor ring\":    (( 1/32) * ( 1/8)) * (10/32) = [  10/8192]\n\
    * \"Ancient coin\":   (( 1/32) * ( 1/8)) * ( 5/32) = [   5/8192]\n\
    * \"Rusty hook\":     (( 1/32) * ( 1/8)) * ( 1/32) = [   1/8192]\n\
    * \"Bloody bandana\": (( 1/32) * ( 1/8)) * ( 1/32) = [   1/8192]\n\
\n\
    A note on unhashable population members:\n\
\n\
    If for whatever reason population members aren't hashable, and they\n\
    must absolutely be used as-is (should never be true in practice...),\n\
    they can be wrapped with xrand.PopEntry, which allows *anything*\n\
    to be used as a dictionary key or set entry, provided it has a\n\
    functioning __eq__ method.\n\
\n\
    PopEntry was designed for a single step when building the population\n\
    for `select`, but since it is the only other way to deal with that\n\
    (hopefully practically non-existent) corner case without sacrificing\n\
    speed and convenience like random.py does by requiring two lists,\n\
    it has been made public.\n\
\n\
    Beware: a way to use lists and dicts as dict keys may sound tempting\n\
    for someone who doesn't understand how hash tables work, but it\n\
    should be said: DO NOT USE IT FOR ANYTHING BESIDES BUILDING CHOICES.\n\
    Since PopEntry calculates the object's hash based on the address of\n\
    its most basic type, multiple unhashables of the same type will all\n\
    go into the same bucket of the hash table. This causes the time to\n\
    lookup a key for anything that hashes to that value to grow\n\
    exponentially.\n\
\n\
    As in, ith just 100 lists, it takes around 11,000x longer\n\
    to check if [0] is in the dict than it would take to see if (0,)\n\
    was in it."
);
PyObject *Random_shuffle(RandomObject *rng, PyObject *arg);
static char Random_shuffle_doc[] = (
"\
    shuffle(list) -> None\n\
    Shuffles a list in-place"
);
PyObject *Random_shuffled(RandomObject *rng, PyObject *arg);
static char Random_shuffled_doc[] = (
"\
    shuffled(iterable) -> list\n\
    Shuffles a new list that was built using the contents of `iterable`"
);
PyObject *Random_split(RandomObject *rng, PyObject *args);
static char Random_split_doc[] = (
"\
    split(n, w) -> [<xrand.Random instance at ...]\n\
    Recursively create new generators based on the current rng's seed\n\
\n\
    `split` creates `n` substream each having a period of 2**64 * w.\n\
    Each substream is guaranteed not to overlap with another substream\n\
    initialized from the same seed.\n\
\n\
    Without the guarantee provided by a PRNG jump function that multiple\n\
    streams won't output the same (or highly correlated) stream of\n\
    pseudorandom numbers, any data generated by more than one processes\n\
    should be considered  highly suspect.\n\
\n\
    Unlike `jump`, this function does not modify the original instance.\n\
\n\
    Note:\n\
    This is as good a place as any to mention that xrand does *not* mess\n\
    with the GIL (global interpreter lock). The underlying generator\n\
    at the C level is already extremely fast (around 1 ns per double) and\n\
    the only remaining bottleneck - converting to/from Python types -\n\
    as a general rule requires the GIL to be held."
);
PyObject *Random_pstate(RandomObject *rng);
static char Random_pstate_doc[] = (
"\
    pstate()\n\
    View the small pool of cached random bits\n\
\n\
    Internally, bits are extracted with one right shift, after which the\n\
    extracted bits are discarded from the pool with a larger left shift.\n\
\n\
    Knowing that (not that there's a reason to this but...) you can\n\
    use the pstate to predict the next values returned by the functions\n\
    which use it.\n\
\n\
    >>> self.pstate\n\
    (1588723712, 17)\n\
    >>> print(f'  {self.pstate[0]:032b}')\n\
    01011110101100100000000000000000\n\
\n\
    >>> # the most significant bits are 01011 which means that we can\n\
    >>> # predict that the next 5 values`flip('a')` returns will be:\n\
    >>> # False(0), 'a'(1), False(0), 'a'(1), 'a'(1)\n\
    >>>\n\
    >>>  for i in range(5):\n\
    print(1 if self.flip(1) else 0, f'{self.pstate[0]:032b}')\n\
    0 10111101011001000000000000000000\n\
    1 01111010110010000000000000000000\n\
    0 11110101100100000000000000000000\n\
    1 11101011001000000000000000000000\n\
    1 11010110010000000000000000000000\n\
    >>> self.pstate\n\
    (3594518528, 12)\n\
    >>> # Since the upper 4 bits are now 0b1101 (13), that will be the\n\
    >>> # next index rolled by `select` when given a sequence of 8-16 items\n\
    >>> seq = \"0123456789abcd\"\n\
    >>> select(range(14))\n\
    'd'\n\
    >>> self.pstate\n\
    (1677721600, 8)"
);
PyObject *Random_seed(RandomObject *rng);
static char Random_seed_doc[] = (
"\
    seed()\n\
    Seed used to initialize the generator"
);
PyObject *Random_state(RandomObject *rng);
static char Random_state_doc[] = (
"\
    state()\n\
    Inspect the current state array of the rng"
);
DATA(PyMethodDef) Random_ml[] = {
        {"__copy__", (PyCFunction)Random___copy__, METH_NOARGS, Random___copy___doc},
        {"__getnewargs_ex__", (PyCFunction)Random___getnewargs_ex__, METH_NOARGS, Random___getnewargs_ex___doc},
        {"choices", (PyCFunction)Random_choices, METH_O, Random_choices_doc},
        {"dice", (PyCFunction)Random_dice, METH_VARARGS, Random_dice_doc},
        {"flip", (PyCFunction)Random_flip, METH_O, Random_flip_doc},
        {"flips", (PyCFunction)Random_flips, METH_VARARGS, Random_flips_doc},
        {"iter_beta", (PyCFunction)Random_iter_beta, METH_VARARGS, Random_iter_beta_doc},
        {"iter_expo", (PyCFunction)Random_iter_expo, METH_VARARGS, Random_iter_expo_doc},
        {"iter_gamma", (PyCFunction)Random_iter_gamma, METH_VARARGS, Random_iter_gamma_doc},
        {"iter_log_normal", (PyCFunction)Random_iter_log_normal, METH_VARARGS, Random_iter_log_normal_doc},
        {"iter_normal", (PyCFunction)Random_iter_normal, METH_VARARGS, Random_iter_normal_doc},
        {"iter_pareto", (PyCFunction)Random_iter_pareto, METH_VARARGS, Random_iter_pareto_doc},
        {"iter_triangular", (PyCFunction)Random_iter_triangular, METH_VARARGS, Random_iter_triangular_doc},
        {"iter_uniform", (PyCFunction)Random_iter_uniform, METH_VARARGS, Random_iter_uniform_doc},
        {"iter_vonmises", (PyCFunction)Random_iter_vonmises, METH_VARARGS, Random_iter_vonmises_doc},
        {"iter_weibull", (PyCFunction)Random_iter_weibull, METH_VARARGS, Random_iter_weibull_doc},
        {"jump", (PyCFunction)Random_jump, METH_NOARGS, Random_jump_doc},
        {"rand_bits", (PyCFunction)Random_rand_bits, METH_O, Random_rand_bits_doc},
        {"rand_bytes", (PyCFunction)Random_rand_bytes, METH_VARARGS, Random_rand_bytes_doc},
        {"rand_float", (PyCFunction)Random_rand_float, METH_NOARGS, Random_rand_float_doc},
        {"rand_index", (PyCFunction)Random_rand_index, METH_O, Random_rand_index_doc},
        {"rand_int", (PyCFunction)Random_rand_int, METH_VARARGS, Random_rand_int_doc},
        {"rv_beta", (PyCFunction)Random_rv_beta, METH_VARARGS, Random_rv_beta_doc},
        {"rv_expo", (PyCFunction)Random_rv_expo, METH_O, Random_rv_expo_doc},
        {"rv_gamma", (PyCFunction)Random_rv_gamma, METH_VARARGS, Random_rv_gamma_doc},
        {"rv_log_normal", (PyCFunction)Random_rv_log_normal, METH_VARARGS, Random_rv_log_normal_doc},
        {"rv_normal", (PyCFunction)Random_rv_normal, METH_VARARGS, Random_rv_normal_doc},
        {"rv_pareto", (PyCFunction)Random_rv_pareto, METH_O, Random_rv_pareto_doc},
        {"rv_triangular", (PyCFunction)Random_rv_triangular, METH_VARARGS, Random_rv_triangular_doc},
        {"rv_uniform", (PyCFunction)Random_rv_uniform, METH_VARARGS, Random_rv_uniform_doc},
        {"rv_vonmises", (PyCFunction)Random_rv_vonmises, METH_VARARGS, Random_rv_vonmises_doc},
        {"rv_weibull", (PyCFunction)Random_rv_weibull, METH_VARARGS, Random_rv_weibull_doc},
        {"sample", (PyCFunction)Random_sample, METH_VARARGS, Random_sample_doc},
        {"select", (PyCFunction)Random_select, METH_O, Random_select_doc},
        {"shuffle", (PyCFunction)Random_shuffle, METH_O, Random_shuffle_doc},
        {"shuffled", (PyCFunction)Random_shuffled, METH_O, Random_shuffled_doc},
        {"split", (PyCFunction)Random_split, METH_VARARGS, Random_split_doc},
        {NULL}
    };
DATA(PyGetSetDef) Random_gs[] = {
        {"pstate", (getter)Random_pstate, (setter)NULL, Random_pstate_doc},
        {"seed", (getter)Random_seed, (setter)NULL, Random_seed_doc},
        {"state", (getter)Random_state, (setter)NULL, Random_state_doc},
        {NULL}
    };
static int Random_init_meths(PyTypeObject *tp) {
    tp->tp_methods = Random_ml;
    tp->tp_getset = Random_gs;
    return PyType_Ready(tp) < 0;
}
PyObject *choices_next(ChoicesObject *ro);
static char choices_next_doc[] = (
"\
    next() -> object\n\
    Calls `select(pop)` using the stored population"
);
PyObject *choices_next_n(ChoicesObject *ro, PyObject *arg);
static char choices_next_n_doc[] = (
"\
    next_n(n) -> object\n\
    Generate a list of `n` random selections"
);
PyObject *choices_sample(ChoicesObject *ro, PyObject *arg);
static char choices_sample_doc[] = (
"\
    sample(k) -> object\n\
    Sample `k` members of the stored population"
);
PyObject *choices_population(ChoicesObject *ro);
static char choices_population_doc[] = (
"\
    population() -> object\n\
    Inspect the choices object's `population`"
);
PyObject *choices_size(ChoicesObject *ro);
static char choices_size_doc[] = (
"\
    size() -> object\n\
    Get the total number of members in the population"
);
DATA(PyMethodDef) choices_ml[] = {
        {"next", (PyCFunction)choices_next, METH_NOARGS, choices_next_doc},
        {"next_n", (PyCFunction)choices_next_n, METH_O, choices_next_n_doc},
        {"sample", (PyCFunction)choices_sample, METH_O, choices_sample_doc},
        {NULL}
    };
DATA(PyGetSetDef) choices_gs[] = {
        {"population", (getter)choices_population, (setter)NULL, choices_population_doc},
        {"size", (getter)choices_size, (setter)NULL, choices_size_doc},
        {NULL}
    };
static int choices_init_meths(PyTypeObject *tp) {
    tp->tp_methods = choices_ml;
    tp->tp_getset = choices_gs;
    return PyType_Ready(tp) < 0;
}
PyObject *dice_next(DiceObject *ro);
static char dice_next_doc[] = (
"\
    next() -> object\n\
    Calls `rand_int` using the stored range`"
);
PyObject *dice_next_n(DiceObject *ro, PyObject *arg);
static char dice_next_n_doc[] = (
"\
    next_n(n) -> object\n\
    Generate a list of `n` random integers based on the stored range"
);
PyObject *dice_max(DiceObject *ro);
static char dice_max_doc[] = (
"\
    max()\n\
    Highest roll of the die"
);
PyObject *dice_min(DiceObject *ro);
static char dice_min_doc[] = (
"\
    min()\n\
    Lowest roll of the dice"
);
DATA(PyMethodDef) dice_ml[] = {
        {"next", (PyCFunction)dice_next, METH_NOARGS, dice_next_doc},
        {"next_n", (PyCFunction)dice_next_n, METH_O, dice_next_n_doc},
        {NULL}
    };
DATA(PyGetSetDef) dice_gs[] = {
        {"max", (getter)dice_max, (setter)NULL, dice_max_doc},
        {"min", (getter)dice_min, (setter)NULL, dice_min_doc},
        {NULL}
    };
static int dice_init_meths(PyTypeObject *tp) {
    tp->tp_methods = dice_ml;
    tp->tp_getset = dice_gs;
    return PyType_Ready(tp) < 0;
}
PyObject *randiter_take(randiter *ri);
static char randiter_take_doc[] = (
"\
    take() -> object\n\
    Call the iterable's __next__ method without consuming an item"
);

DATA(PyMethodDef) randiter_ml[] = {
        {"take", (PyCFunction)randiter_take, METH_NOARGS, randiter_take_doc},
        {NULL}
    };
DATA(PyGetSetDef) randiter_gs[] = {{NULL}};
static int randiter_init_meths(PyTypeObject *tp) {
    tp->tp_methods = randiter_ml;
    tp->tp_getset = NULL;
    return PyType_Ready(tp) < 0;
}
PyObject *setobject___sizeof__(setobject *so);
static char setobject___sizeof___doc[] = (
"\
    __sizeof__()\n\
    size of self in bytes"
);
PyObject *setobject_add(setobject *so, PyObject *arg);
static char setobject_add_doc[] = (
"\
    add(integer)\n\
    Add an integer to the set"
);
PyObject *setobject_update(setobject *so, PyObject *arg);
static char setobject_update_doc[] = (
"\
    update(integers)\n\
    Add some integers to the set"
);
PyObject *setobject_mask(setobject *so);
static char setobject_mask_doc[] = (
"\
    mask()\n\
    The size of the hash table (where `size` is the max entries)"
);
DATA(PyMethodDef) setobject_ml[] = {
        {"__sizeof__", (PyCFunction)setobject___sizeof__, METH_NOARGS, setobject___sizeof___doc},
        {"add", (PyCFunction)setobject_add, METH_O, setobject_add_doc},
        {"update", (PyCFunction)setobject_update, METH_O, setobject_update_doc},
        {NULL}
    };
DATA(PyGetSetDef) setobject_gs[] = {
        {"mask", (getter)setobject_mask, (setter)NULL, setobject_mask_doc},
        {NULL}
    };
static int setobject_init_meths(PyTypeObject *tp) {
    tp->tp_methods = setobject_ml;
    tp->tp_getset = setobject_gs;
    return PyType_Ready(tp) < 0;
}