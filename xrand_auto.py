from collections import deque
from itertools import starmap
from builtins import min as _min, max as _max
import string
from itertools import *
from operator import *
from bitecodes import Signature
import sys


PYCFUNC = 'PyCFunction'
PYSETTER= 'setter'
PYGETTER= 'getter'
NULL = 'NULL'
INVALID_PARSER_FUNC = 'invalid_parser_spec'

# nwe parsers must be defined here
# o - object (no incref)
# d - object to double
# L - object to PyLongObject via __int__ method
# K - object to PyLongObject via __index_ method
# i - object to int64_t
# I - object to int64_t and overflow errors are ignored
# u - object to uint64_t
# U - object to uint64_t and overflow errors are ignored
# n - object to ssize_t
# N - object to size_t
# T - object to 1 or 0 (as "int") based on object.__bool__
# s - object to PySequence; object *must* be a sequence type
# S - object to PySequence; object can be any iterable
FORMATS = 'odLKiIuUnNTsS'
METHODS = []
UNWRAPPED = deque()

Random = 'Random'
choices = 'choices'
dice = 'dice'
randiter = 'randiter'
setobject = 'setobject'
Module = None
FLAGS = {0:'METH_NOARGS',
         1:'METH_O',
         2:'METH_VARARGS',
         3:'METH_VARARGS|METH_KEYWORDS'
         }
PROTOS={0:'PyObject *{name}({obj} *{this})',
        1:'PyObject *{name}({obj} *{this}, PyObject *arg)',
        2:'PyObject *{name}({obj} *{this}, PyObject *args)',
        3:'PyObject *{name}({obj} *{this}, PyObject *args, PyObject *kws)'
        }
METHOD_DEFS = {}
DESCR_DEFS = {}
AUTO = {}
SELF_TYPES = {Random:'RandomObject',
              choices: 'ChoicesObject',
              dice: 'DiceObject',
              randiter: 'randiter',
              setobject: 'setobject',
              Module:'PyObject'}
SELF_NAMES = {Random: 'rng',
              choices: 'ro',
              dice: 'ro',
              randiter: 'ri',
              setobject: 'so',
              Module:'self'}
CHARMAP =  {**{i: bytes([i]) for i in range(32, 127)},
            **{i: b'\\%02x'%i for i in [*range(32),*range(127, 256)]}
            }
CHARMAP[ord('"')] = b'\\"'
CHARMAP[ord('\n')] = [ord('\n')]
CHARMAP[ord('\r')] = [ord('\t')]
CHARMAP[ord('\t')] = [ord('\t')]

def implicit_concat_repr(s, encoding=None, indentation=4):
    '''Convert a Python unicode string into an ascii-only representation
    suitable for use as string literals in programming languages that
    only accept implicitly concatenated, ascii-only string literals.
    '''
    if encoding is None:
        encoding = 'utf-8'
    b = s.strip().encode(encoding)
    a = bytearray()
    f = a.extend
    for char in b:
        f(CHARMAP[char])
    lines =[i.lstrip() for i in a.decode().split('\n')]
    r = []
    f = r.append
    pad = ' '*indentation
    for line in lines:
        if line:
            line = f'{pad}{line}'
        f(line)
    r = '\\n\\\n'.join(r)
    return f'"\\\n{r}"'

implicit_concat_repr.print = lambda: print(
    implicit_concat_repr(implicit_concat_repr.__doc__))
DOCTYPE = 'char' if sys.version_info < (3, 7) else 'const char'
class PyMethodDef:
    "Generates the method's prototype, doc, and PyMethodDef struct"

    def __init__(self, cls, name, doc, tp, sig=None, descr=False):
        self.py_type = cls
        self.meth_type = tp
        obj = SELF_TYPES[cls]
        if doc is not None:
            if sig is not None:
                doc = sig._repr_str + '\n' + doc
        doc = self.doc = implicit_concat_repr('' if doc is None else doc)
        self.fname = name
        METHODS.append(name)
        this = self.this = SELF_NAMES[cls]
        name = self.name = f'{cls}_{name}' if cls is not None else name
        self.proto = PROTOS[tp].format_map(vars())
        self.proto_def = (f'#define {self.ml_proto}(...) '
                          f'{self.proto}')
        #self.doc_def = f'static char {self.ml_doc}[] = (\n{doc}\n);'
        self._is_descr = descr
        if descr:
            #self.descr_def = f'#define {self.ml_def} {self.repr}'
            DESCR_DEFS.setdefault(cls, {})[self.fname] = self
        else:
            self.flags_def = f'#define {self.ml_flags} {FLAGS[tp]}'
            
            self.method_def = f'#define {self.ml_def} {self.repr}'
            METHOD_DEFS.setdefault(cls, {})[self.fname] = self

    @property
    def doc_def(self):
        return f'static {DOCTYPE} {self.ml_doc}[] = (\n{self.doc}\n);'
    @property
    def signature(self):
        return getattr(self, '_sig', None)

    @property
    def docstring(self):
        if self.signature is not None:
            return f'{self._sig._repr_str}\n{self.doc}'
        return self.doc
    @property
    def flags(self):
        return FLAGS[self.meth_type]

    @property
    def new_ml(self):
        return (
            f'{self.proto};\n'
            f'static {DOCTYPE} {self.ml_doc}[] = (\n{self.doc}\n);')
            
            
    @property
    def ml(self):
        return (self.method_def,
                self.flags_def,
                self.proto_def,
                f'{self.ml_proto}();',
                self.doc_def)
    @property
    def struct(self):
        return SELF_TYPES[self.py_type]

    @property
    def ml_flags(self):
        return f'{self.name}_flags'

    @property
    def ml_name(self):
        return f'"{self.fname}"'

    @property
    def ml_doc(self):
        return f'{self.name}_doc'

    @property
    def gg_func(self):
        return f'({PYGETTER}){self.name}'

    @property
    def gs_func(self):
        return f'({PYSETTER})NULL'
    
    @property
    def ml_func(self):
        return f'({PYCFUNC}){self.name}'

    @property
    def get_func(self):
        return f'(getter){self.name}'
    
    @property
    def ml_def(self):
        return f'{self.name}_def'

    @property
    def ml_proto(self):
        return f'{self.name}_proto'

    @property
    def repr(self):
        if self._is_descr:
            return (f'{{{self.ml_name}, '
                    f'{self.get_func}, '
                    f'NULL, '
                    f'{self.ml_doc}}}')
                    
        return (f'{{{self.ml_name}, '
                f'{self.ml_func}, '
                f'{self.ml_flags}, '
                f'{self.ml_doc}}}')

    @classmethod
    def from_func(cls, f, descr, prefix=None):

        flags = f.__code__.co_flags
        has_args = flags & 4
        has_kws = flags & 8
        if has_kws:
            if not has_args:
                raise TypeError("cannot have kwargs without args")
            n = 3
        elif has_args:
            n = 2
        else:
            n = _min(2, f.__code__.co_argcount)
        sig=Signature(f)
        r = PyMethodDef(prefix, f.__name__, f.__doc__, n, sig,
                        descr=descr)
        r._func = f
        r._sig = sig
        return r

def descdef(cls=None):
    return docdef(cls, False)

def docdef(cls=None, is_method=True):

    def apply(f):
        return PyMethodDef.from_func(f, not is_method, cls)

    if isinstance(cls, str):
        return apply

    def executor():
        nonlocal cls
        f = cls
        cls = None
        return apply(f)
    UNWRAPPED.append(executor)
    
    return

@docdef(setobject)
def add(integer):
    'Add an integer to the set'

@descdef(setobject)
def mask():
    'The size of the hash table (where `size` is the max entries)'

@docdef(setobject)
def __sizeof__():
    'size of self in bytes'
    
@docdef(setobject)
def update(integers):
    'Add some integers to the set'
    
@docdef(choices)
def next() -> object:
    'Calls `select(pop)` using the stored population'

@docdef(choices)
def next_n(n) -> object:
    'Generate a list of `n` random selections'

@docdef(choices)
def sample(k) -> object:
    'Sample `k` members of the stored population'

@descdef(choices)
def population() -> object:
    '''Inspect the choices object's `population`'''
@descdef(choices)
def size() -> object:
    '''Get the total number of members in the population'''
@docdef(dice)
def next() -> object:
    'Calls `rand_int` using the stored range`'

@docdef(dice)
def next_n(n) -> object:
    'Generate a list of `n` random integers based on the stored range'
@descdef(dice)
def max():
    'Highest roll of the die'
@descdef(dice)
def min():
    'Lowest roll of the dice'
@docdef(randiter)
def take() -> object:
    "Call the iterable's __next__ method without consuming an item"

t4='''FUNC(Py) randiter_{f}_repr(randiter *ri) {
    sizes_t _ctr_ = {randiter_COUNTER(ri)};
    {f}variate_args_t *a = &_riter_stat_args_(ri).{f};
    {newrefs}
    Py result = __format__("rv_{f}(%zu, {spec})", ctr, {sig});
    {cleanup}
    return result;
}'''

rv_autorepr_temp='''FUNC(Py) randiter_{name}_repr(randiter *ri) {{

    Py result = NULL;
{ob_defs}
{build_args}
{build_obs}
    result = {ret}
    CLEAN:
{cleanup}
    return result;
}}'''
_AUTO_ARGS_ = '_ARGS_'
def new_ob_def(attr):
    return f'    Py {attr} = NULL;'
def build_ob(attr, f):
    return (f'    if(!({attr} = {f}({_AUTO_ARGS_} ->{attr}))) goto CLEAN;')
def dec_ob_def(attr):
    return f'    Py_XDECREF({attr});'
def new_args(f):
    return f'    {f}variate_args_t *{_AUTO_ARGS_}  = &_riter_stat_args_(ri).{f};'
def new_result(m):
    s = m.signature
    args = s.args[1:]
    spec = ", ".join(['%R'] * len(args))
    sig = ', '.join(args)
    ctr = 'randiter_COUNTER(ri)'
    return f'__format__("{m.fname}(%zi, {spec})", {ctr}, {sig});'
def rv_autorepr(Name, m, *converters):
    global AUTOERR
    name = Name + ''
    
    s = m.signature
    assert s.args[0] == 'n'
    args = s.args[1:]
    if len(args) != len(converters):
        AUTOERR = name, m, converters
        assert 1==0
    assert len(args) == len(converters)
    ob_defs = '\n'.join(map(new_ob_def, args))
    build_args =new_args(Name)
    build_obs = '\n'.join(map(build_ob, args, converters))
    ret = new_result(m)
    
    cleanup = '\n'.join(map(dec_ob_def, args))
    mp = vars()
    return rv_autorepr_temp.format_map(mp)
RV_AUTOREPRS = {}
GET_AUTOCONVERT = {
    'd':'__float__',
    'n':'py_int_from_ssize_t',
    'N':'py_int_from_size_t',
    's':'__format__'
}.__getitem__
def has_rv_autorepr(name, *verters):
    assert isinstance(name, str), 'missing parentheses'
    converters = map(GET_AUTOCONVERT, verters)
    def wrapper(m):
        RV_AUTOREPRS[name] = rv_autorepr(name, m, *converters)
        return m
    RV_AUTOREPRS[name] = None
    return wrapper
def save_autoreprs(fp):
    src = ['#ifndef RV_AUTOREPRS_H',
           '#define RV_AUTOREPRS_H',
           '#define lognormvariate_args_t normalvariate_args_t',
           '#define normvariate_args_t normalvariate_args_t',
           '#define uniformvariate_args_t uniform_args_t',
           
           ]
    for name, d in sorted(RV_AUTOREPRS.items()):
        src.append(d)
    src.append('#endif')
    code = '\n'.join(src)
    code = code.replace('lamda', 'lambda')
    fp.write(code)
@docdef(Random)
def rv_beta(alpha, beta) -> float:
    'Random variable with beta distribution'

@has_rv_autorepr('beta', 'd','d')
@docdef(Random)
def iter_beta(n, alpha, beta) -> randiter:
    'Iterate over` n` random variables with beta distribution' '''
    If `n` is None the iterator will repeat forever.

    The iterator's `take` method can call `__next__` without
    reducing the number of remaining iterations and it can be used
    even after the iterator has been exhausted.
    '''
    
@docdef(Random)
def rv_expo(lamda) -> float:
    'Random variable with exponential distribution'

@has_rv_autorepr('expo', 'd')
@docdef(Random)
def iter_expo(n, lamda) -> randiter:
    'Iterate over` n` random variables with exponential distribution' '''
    If `n` is None the iterator will repeat forever.

    The iterator's `take` method can call `__next__` without
    reducing the number of remaining iterations and it can be used
    even after the iterator has been exhausted.
    '''

    
@docdef(Random)
def rv_gamma(alpha, beta)->float:
    'Random variable with gamma distribution'
@has_rv_autorepr('gamma', 'd','d')
@docdef(Random)
def iter_gamma(n, alpha, beta)->randiter:
    'Iterate over` n` random variables with gamma distribution' '''
    If `n` is None the iterator will repeat forever.

    The iterator's `take` method can call `__next__` without
    reducing the number of remaining iterations and it can be used
    even after the iterator has been exhausted.
    '''

    
@docdef(Random)
def rv_log_normal(mu, sigma) -> float:
    'Random variable with log normal distribution'
@has_rv_autorepr('lognorm', 'd','d')
@docdef(Random)
def iter_log_normal(n, mu, sigma) -> randiter:
    'Iterate over` n` log-normally distributed random variables' '''
    If `n` is None the iterator will repeat forever.

    The iterator's `take` method can call `__next__` without
    reducing the number of remaining iterations and it can be used
    even after the iterator has been exhausted.
    '''

    
@docdef(Random)
def rv_normal(mu, sigma) ->float:
    'Random variable with normal (Gaussian) distribution'
@has_rv_autorepr('norm', 'd', 'd')
@docdef(Random)
def iter_normal(n, mu, sigma) ->randiter:
    'Iterate over` n` normally distributed random variables' '''
    If `n` is None the iterator will repeat forever.

    The iterator's `take` method can call `__next__` without
    reducing the number of remaining iterations and it can be used
    even after the iterator has been exhausted.
    '''

    
@docdef(Random)
def rv_pareto(alpha) ->float:
    'Random variable with Pareto distribution'
@has_rv_autorepr('pareto', 'd')
@docdef(Random)
def iter_pareto(n, alpha) ->randiter:
    'Iterate over` n` random variables with Pareto distribution' '''
    If `n` is None the iterator will repeat forever.

    The iterator's `take` method can call `__next__` without
    reducing the number of remaining iterations and it can be used
    even after the iterator has been exhausted.
    '''
    
@docdef(Random)
def rv_triangular(low, high, mode) -> float:
    'Random variable with triangular distribution'
@has_rv_autorepr('triangular', 'd', 'd', 'd')
@docdef(Random)
def iter_triangular(n, lo, hi, c) -> randiter:
    'Iterate over` n` random variables with triangular distribution' '''
    If `n` is None the iterator will repeat forever.

    The iterator's `take` method can call `__next__` without
    reducing the number of remaining iterations and it can be used
    even after the iterator has been exhausted.
    '''


@docdef(Random)
def rv_vonmises(mu, kappa) -> float:
    'Random variable with Vonmises distribution'
@has_rv_autorepr('vonmises', 'd','d')
@docdef(Random)
def iter_vonmises(n, mu, kappa) -> float:
    'Iterate over` n` random variables with Vonmises distribution' '''
    If `n` is None the iterator will repeat forever.

    The iterator's `take` method can call `__next__` without
    reducing the number of remaining iterations and it can be used
    even after the iterator has been exhausted.
    '''
    

@docdef(Random)
def rv_weibull(alpha, beta) -> float:
    'Random variable with Weibull distribution'
@has_rv_autorepr('weibull','d','d')
@docdef(Random)
def iter_weibull(n, alpha, beta) -> float:
    'Iterate over` n` random variables with Weibull distribution' '''
    If `n` is None the iterator will repeat forever.

    The iterator's `take` method can call `__next__` without
    reducing the number of remaining iterations and it can be used
    even after the iterator has been exhausted.
    '''


@docdef(Random)
def rv_uniform(a, b) -> float:
    'Random float from a uniform distribution [a, b)' 
@has_rv_autorepr('uniform','d','d')
@docdef(Random)
def iter_uniform(n, a,b)->float:
    'Iterate over `n` random variables from the uniform distribution [a,b)' '''
    If `n` is None the iterator will repeat forever.

    The iterator's `take` method can call `__next__` without
    reducing the number of remaining iterations and it can be used
    even after the iterator has been exhausted.
    '''
    
@docdef(Random)
def select(population) -> object:
    "Select a random member of `population`"'''

    `population` may be any iterable or dict/xrand.Population instance.

    If `population` is an iterator/sequence, each member of the
    population has a uniform (1/length) chance to be selected.

    If `population` is a dict or `xrand.Population` instance, the
    "keys" represent the population population members and the "values"
    represent the weighting given to that particular member.
    
    Member weightings may be represented as floats or integers.
    However, all weightings for any given population must have be of
    the same type.

    This dict based population is invalid since it mixes ints and floats:

    >>> {"a":1.0, "b":2.0, "c":1}

    The keys in both of the following dicts will end up having roughly
    the same odds of chosen:

    >>> i_based = {"a":  1, "b":   2, "c":   7}
    >>> f_based = {"a":0.5, "b": 1.0, "c": 3.5}

    # Only "roughly" the same odds because floats are inherently
    # imprecise (although insignificantly so, in this case)

    - "a" will have a 1/10 (0.5/5.0) chance of being selected
    - "b" will have a 2/10 (1.0/5.0) chance of being selected
    - "c" will have a 7/10 (3.5/5.0) chance of being selected

    `Random.choices` objects are special population members.
    If the member chosen happens to be a `choices` instance
    its `next` method is automatically (and recursively) invoked.

    This could be used to implement and NPC drop table on an RPG
    which could look something like this:

    >>> coins_table = choices(range(200, 450))
    >>> SCARY_PIRATE_DROPTABLE = {
        None: 16,
        coins_table: 12,
        choices({
            "Health potion":15,
            "Elixir of life":1
        }): 3,
        choices({
            'Eyepatch':6,
            'Deadly cutlass':1,
            choices({
                'Treasure box':15,
                'Meteor ring':10,
                'Ancient coin':5,
                'Rusty hook':1,
                'Bloody bandana':1
            }):1
        }): 1}

    >>> npc.get_loot = choices(SCARY_PIRATE_DROPTABLE).next

    `npc.get_loot()` will return items with the following drop rates:

#                       roll #1   roll #2   roll #3    effective
    * None:             ( 16/32)                     = [4096/8192]
    * 200-450 coins:    ( 12/32)                     = [3072/8192]
    * "Health potion":  (( 3/32) * (15/16))          = [ 720/8192]
    * "Elixir of life": (( 3/32) * ( 1/16))          = [  48/8192]
    * "Eyepatch":       (( 1/32) * ( 6/8))           = [ 192/8192]
    * "Deadly cutlass": (( 1/32) * ( 1/8))           = [  32/8192]
    * "Treasure box":   (( 1/32) * ( 1/8)) * (15/32) = [  15/8192]
    * "Meteor ring":    (( 1/32) * ( 1/8)) * (10/32) = [  10/8192]
    * "Ancient coin":   (( 1/32) * ( 1/8)) * ( 5/32) = [   5/8192]
    * "Rusty hook":     (( 1/32) * ( 1/8)) * ( 1/32) = [   1/8192]
    * "Bloody bandana": (( 1/32) * ( 1/8)) * ( 1/32) = [   1/8192]

    A note on unhashable population members:

    If for whatever reason population members aren't hashable, and they
    must absolutely be used as-is (should never be true in practice...),
    they can be wrapped with xrand.PopEntry, which allows *anything*
    to be used as a dictionary key or set entry, provided it has a
    functioning __eq__ method.

    PopEntry was designed for a single step when building the population
    for `select`, but since it is the only other way to deal with that
    (hopefully practically non-existent) corner case without sacrificing
    speed and convenience like random.py does by requiring two lists,
    it has been made public.

    Beware: a way to use lists and dicts as dict keys may sound tempting
    for someone who doesn't understand how hash tables work, but it
    should be said: DO NOT USE IT FOR ANYTHING BESIDES BUILDING CHOICES.
    Since PopEntry calculates the object's hash based on the address of
    its most basic type, multiple unhashables of the same type will all
    go into the same bucket of the hash table. This causes the time to
    lookup a key for anything that hashes to that value to grow
    exponentially.

    As in, ith just 100 lists, it takes around 11,000x longer
    to check if [0] is in the dict than it would take to see if (0,)
    was in it.
    '''

def choices(population) -> 'choices':
    "Get a new 'choices' object based on `population`" '''

    On every function call before it can actually make a `choice`,
    `Random.choice` must first convert the abstract Python object form
    `population` is stored as into a machine-readable representation.

    This process of converting Python objects to more fundamental data
    types is oftentimes the most expensive segment of a function call.
    Thus, keeping the arguments in their most fundamental types can
    significantly increase performance if multiple function calls
    taking the same arguments is needed, especially if all iteration
    is done at the C level.

    `choices` objects have the following methods:
    
        * next():
            Identical to:
                >>> f = partial(rng.select, population)
                >>> f()
                
        * next_n(n):
            Identical to:
                >>> f = partial(rng.select, population)
                >>> [f() for i in range(n)]
                
        * sample(k):
            Identical to:
                >>> f= partial(rng.sample, population)
                >>> f(k)
            Note: only between 5-10% faster than sample. Including
            `choices` construction time, takes many calls to reach
            parity with just using `sample`

     along with the following data descriptors:
     
        * ro.population:
            The population being stored. For dict based `choices`
            objects it returns two lists; the first are all unique
            elements. The other is a list equivalent to
            accumulate(p.values()).

            For ranges, strings, tuples, and lists, the original
            sequence is returned. Everything else will have been
            converted to a  list.
            
        * ro.size:
            Population size. For dicts, this is the effective count
            of  the members in the population (sum(pop.values())).
            For everything else it is simply the length of whatever
            sequence representation `choices`  decided to convert it to.

    The caveats for `Random.select` and `Random.sample` still apply.

    Here's some Python level benchmarking between this and random.py:
    (number of random.choice(pop) equivalent calls per second)

    >>> from stex import e
    >>> from random import choice as pychoice, sample as pysample
    >>> from xrand import select, choices, sample
    >>> pop = range(123456)
    >>> r = e('pychoice(pop)'); print(f'{r:_.0f}')
    654_141
    >>> r = e('select(pop)'); print(f'{r:_.0f}')
    8_392_386
    >>> f = choices(pop).next; r = e('f()'); print(f'{r:_.0f}')
    11_400_663 
    >>> r = e('[pychoice(pop) for i in range(100)]')*100; print(f'{r:_.0f}')
    644_486
    >>> r = e('[select(pop) for i in range(100)]')*100; print(f'{r:_.0f}')
    6_920_097
    >>> f = choices(pop).next_n; r = e('f(100)')*100; print(f'{r:_.0f}')
    22_869_879
    
    >>> cards = itertools.product('HSDC',(*'23456789','10',*'JKQA'))
    >>> cards = [f'{a}{b}' for a,b in cards]
    >>> r = e('pysample(cards, 20)'); print(f'{r:_.0f}')
    29_768
    >>> r = e('sample(cards, 20)'); print(f'{r:_.0f}')
    1_165_779
    
    `select(pop)`  -> 13x faster
    `.next()       -> 17x faster
    `.next_n(100)` -> 35x faster
    `sample(cards) -> 39x faster (to be fair, varies 20-50x)

    '''
cd = choices.__doc__
choices = docdef(Random)(choices)

@docdef(Random)
def dice(a=0, b=0) -> 'dice':
    "Get a new `dice' object that rolls random ints from [a, b]" '''

    This object has 2 methods:

        * next():
            Identical to:
                >>> f = partial(rng.rand_int, a, b)
                >>> f()
                
        * next_n(n):
            Identical to:
                >>> f = partial(rng.select, a, b)
                >>> [f() for i in range(n)]
                
     along with the following data descriptors:
     
        * ro.max:
            Highest roll of the dice

        * ro.min:
            Lowest roll of the dice

    '''

@docdef(Random)
def flip(heads_value) ->object:
    '50% chance to return either `heads_value` or False'

@docdef(Random)
def flips(n, heads_odds=0.5, heads=True, tails=False):
    "Iterator producing `n` binary choices" '''

    Every loop has a `heads_odds` chance to return `heads which
    means it has a `(1.0 - heads_odds)` chance to return `tails.

    If `n` is None the iterator will repeat forever.

    The iterator's `take` method can call `__next__` without
    reducing the number of remaining iterations and it can be used
    even after the iterator has been exhausted.
    '''

@docdef(Random)
def rand_bits(bits) ->int:
    'Random int from uniformly-distributed range [0, 2**bits)'

@docdef(Random)
def jump() -> None:
    'Advances the underlying generator state by 2**64 iterations'


@docdef(Random)
def rand_bytes(size, buf=None) ->object:
    'Generates `size` random bytes' '''

    With only the `size` argument, return a new bytes object filled
    with `size` random bytes. If the optional `buf` argument is
    supplied, it must be an object supporting the buffer protocol.
    A type error will be raised if the buffer is not C-style contiguous
    or is read-only.    

    Examples:
    >>> rand_bytes(5)
    b'abcde'

    >>> x = array.array('Q', [0])
    >>> rand_bytes(8, x)
    array([10583363146662955482], 'Q')

    >>> import numpy as np
    >>> x = rand_bytes(3*4, np.zeros(3, 'i'))
    >>> y = rand_bytes(len(x)*4, x)
    >>> x is y, y # note the original object is modified...
    (True, array([-1338281059, -1828093642,  1742631451], dtype=int32))

    '''
@docdef(Random)
def rand_index(i) ->int:
    'Random int from the uniformly distributed range [0, i)' '''

    `i` must be positive and able to fit in a platform size_t.
    The exact maximum value may be seen from within python with:

    >>> from ctypes import *; 2**(sizeof(c_size_t) * 8)-1

    In general this is 2**64-1 for 64 and 2**32 for 32 bit platforms.

    Because the result is from the half-open interval [0, i) and
    since 2**bits-1 is the maximum value `i` can be without an overflow
    error, 2**bits-1 can never be a return value from this function.
    '''

@docdef(Random)
def rand_int(a, b=None) ->int:
    "Random int from the uniformly-distributed range [a, b]" '''

    As with `rv_uniform`, the order of `a` and `b` is of no consequence:

    All four of the following function calls are identical:
    
    >>> rand_int(-10)
    >>> rand_int(0, -10)
    >>> rand_int(-10, 0)
    >>> select(range(-10, 1)

    Keep in mind that just the presence of a second argument causes
    an extra 20% overhead to come out of nowhere, even if it's as
    simple as the difference between rand_int(0, 20) and rand_int(20)

    Also see `Random.dice`
    
    '''

@docdef(Random)
def rand_float() -> float:
    'Random float from uniformly distributed half-open interval [0, 1.0)'

@docdef(Random)
def sample(pop, k) -> list:
    'Choose `k` elements from the `pop`ulation without replacement'"""

    
    If `pop` is a dictionary, the dict keys represent the members
    of the populations and the corresponding values represent the
    number of members in the population. Unlike `select` which allows
    both int and float-based weighting, `sample` only acceptes ints.
    
    When `pop` is a sequence or iterable, each *index* can only be
    chosen once, which means that duplicates just increase the odds
    that member will be chosen.


    "Replacement" means that if the population is "abcde" for example,
    once "a" is chosen it can never be chosen again. Internally, since
    what is actually chosen is the element's index, what it really
    means it that the number `0` out of the total population size (5)
    will never be chosen again. This also applies when weightings
    are provided. {"a": 4, "b":1} is logically converted to the list
    ["a", "a", "a", "a", "b"] so that up to 4 "a" can be selected.
    
    Example:

    >>> sample(range(52))
    [44, 18, 4, 13, 33]
    >>> pop = Counter("aaabbbccc")
    >>> pop
    Counter({"a":3, "b":3, "c":3})
    >>> sample(pop, 6)
    ['c', 'a', 'c', 'c', 'b', 'a']

    The Counter example is functionally identical to:
    
    >>> random.sample(list(pop.elements()), 5)

    While sampling from a tuple or list will perform about 50% faster,
    in the right circumstances (a large population with few uniques),
    the dict based form will use orders of magnitude less memory.

    >>> pop = ''.join(choices(string.ascii_lowercase).next_n(100000))
    >>> popc= Counter(pop)

    `pop` in the above example requires 400-800KB of memory whereas
    `popc` only requires about 1KB.

    See the documentation for `choices` and `select` for more info,
    including how to handle the corner case of a weighted population
    that has unhashable members.
    """

@docdef(Random)
def shuffle(list) ->None:
    'Shuffles a list in-place'

@docdef(Random)
def shuffled(iterable) -> list:
    'Shuffles a new list that was built using the contents of `iterable`'

@docdef(Random)
def split(n, w) -> '[<xrand.Random instance at ...]':
    "Recursively create new generators based on the current rng's seed" '''

    `split` creates `n` substream each having a period of 2**64 * w.
    Each substream is guaranteed not to overlap with another substream
    initialized from the same seed.

    Without the guarantee provided by a PRNG jump function that multiple
    streams won't output the same (or highly correlated) stream of
    pseudorandom numbers, any data generated by more than one processes
    should be considered  highly suspect.

    Unlike `jump`, this function does not modify the original instance.

    Note:
    This is as good a place as any to mention that xrand does *not* mess
    with the GIL (global interpreter lock). The underlying generator
    at the C level is already extremely fast (around 1 ns per double) and
    the only remaining bottleneck - converting to/from Python types -
    as a general rule requires the GIL to be held.
    '''

@descdef(Random)
def seed():
    'Seed used to initialize the generator'

@descdef(Random)
def pstate():
    'View the small pool of cached random bits' """

    Internally, bits are extracted with one right shift, after which the
    extracted bits are discarded from the pool with a larger left shift.
    
    Knowing that (not that there's a reason to this but...) you can
    use the pstate to predict the next values returned by the functions
    which use it.
    
    >>> self.pstate
    (1588723712, 17)
    >>> print(f'  {self.pstate[0]:032b}')
      01011110101100100000000000000000
      
    >>> # the most significant bits are 01011 which means that we can
    >>> # predict that the next 5 values`flip('a')` returns will be:
    >>> # False(0), 'a'(1), False(0), 'a'(1), 'a'(1)
    >>>
    >>>  for i in range(5):
            print(1 if self.flip(1) else 0, f'{self.pstate[0]:032b}')
    0 10111101011001000000000000000000
    1 01111010110010000000000000000000
    0 11110101100100000000000000000000
    1 11101011001000000000000000000000
    1 11010110010000000000000000000000
    >>> self.pstate
    (3594518528, 12)
    >>> # Since the upper 4 bits are now 0b1101 (13), that will be the
    >>> # next index rolled by `select` when given a sequence of 8-16 items
    >>> seq = "0123456789abcd"
    >>> select(range(14))
    'd'
    >>> self.pstate
    (1677721600, 8)
"""

@descdef(Random)
def state():
    'Inspect the current state array of the rng'

@docdef(Random)
def __getnewargs_ex__():
    'Helper for pickle' '''

    The constructor for Random takes 1 or 0 arguments. If it gets an
    argument that is a tuple it assumes it's supposed to unpickle it.
    There's no reason for this output to be human readable but it might
    as well be documented here what the garbage __getnewargs_ex__
    returns represents...

    >>> s0, s1 = rng.state
    >>> seed = rng.seed
    >>> p_val, p_bits = rng.pstate
    
    >>> import struct
    >>> packed = struct.pack('QQ16sNN', s0, s1, seed, p_val, p_bits)
    >>> pickled = struct.unpack('32sNN', packed)
    >>> new = Random(pickled).state
    >>> new.state == rng.state and new.pstate == rng.pstate
    True
    >>> rng.flip('h')
    False
    >>> new.state == rng.state and new.pstate == rng.pstate
    False    
'''

    
@docdef(Random)
def __copy__():
    'Get a new copy of `self`'
    
def get_parser():
    case = lambda a, b, c: f'({a}? {b}:\n{c})'
    tail = INVALID_PARSER_FUNC
    for ch in reversed(FORMATS):
        tail = case(f"v=='{ch}'", f'arg_parse_{ch}', tail)
    result = f"{case(' !v   ', 'NULL', tail)}".split('\n')
    return define('GET_PARSER(v)', *result)

def define(mname, *args):
    if len(args) > 1:
        args = ('',) + args
    mdef = "\\\n".join(args)
    return f'#define {mname} {mdef}'

def aliases(*,bint='enum _bint',
            py_bytes='PyBytesObject',
            py_dict='PyDictObject',
            py_float='PyFloatObject',
            py_int='PyLongObject',
            py_list='PyListObject',
            py_set='PySetObject',
            py_tuple='PyTupleObject',
            py_type='PyTypeObject',
            py_unicode='PyUnicodeObject'):
    return  sorted(starmap(define, aliases.__kwdefaults__.items()))

def auto_begin():
    while UNWRAPPED:
        UNWRAPPED.popleft()()
    return  [f'static const char module_doc[] = (\n'
             f'{implicit_concat_repr(MODULE_DOC)}\n'
             f');',
             *aliases(),
             get_parser(),
             ]

reprgetter=attrgetter('repr')
sorted_dict_values = lambda d: map(d.__getitem__, sorted(d))
join_lines = lambda lines: ',\n'.join(lines)
Lpad = lambda n, s: f'{" "*4*n}' f'{s}'
ML = lambda cls, m: METHOD_DEFS[cls][m]
DL = lambda cls, m: DESCR_DEFS[cls][m]
def TYPE_INIT(cls):
    mldef = gsdef = NULL
    mds = DESCR_DEFS.get(cls)
    mls = METHOD_DEFS.get(cls)
    def get_meth_def(meth): # gg_func, gs_func, ml_func (PyCF)x
        return ('{'
                f'{meth.ml_name}, '
                f'{meth.ml_func}, '
                f'{meth.flags}, '
                f'{meth.ml_doc}'
                '}')
    def get_desc_def(meth):
        return ('{'
                f'{meth.ml_name}, '
                f'{meth.gg_func}, '
                f'{meth.gs_func}, '
                f'{meth.ml_doc}'
                '}')
    def unpack(defgetter, mds):
        if not mds:
            return '{{NULL}}'
        
        mds  = sorted_dict_values(mds)
        lines = map(defgetter, mds)
        lines = [*lines, '{NULL}']
        lines = map(Lpad, repeat(2),lines) 
        return '{\n' f'{join_lines(lines)}' '\n    }'

    d = DESCR_DEFS.get(cls, {})
    val = 'NULL' if not d else f'{cls}_gs'
    set_getsets = f'    tp->tp_getset = {val};\n'
    gsdef = unpack(get_desc_def, d)
    
    m = METHOD_DEFS.get(cls, {})
    val = 'NULL' if not m else f'{cls}_ml'
    set_methods = f'    tp->tp_methods = {val};\n'
    mldef = unpack(get_meth_def, m)
    return (
        f'DATA(PyMethodDef) {cls}_ml[] = {mldef};' '\n'
        f'DATA(PyGetSetDef) {cls}_gs[] = {gsdef};' '\n'
        f'static int {cls}_init_meths(PyTypeObject *tp)' ' {\n'        
        f'{set_methods}' 
        f'{set_getsets}'
        f'    return PyType_Ready(tp) < 0;''\n}')   
def auto2():
    'Build auto.h which contains method prototypes and type initializers'
    lines = auto_begin()
    add_line = lines.append
    get_new_ml = attrgetter('new_ml')
    for cls, methods in sorted(METHOD_DEFS.items()):
        add_line('\n'.join(map(get_new_ml, sorted_dict_values(methods))))
        descrs = DESCR_DEFS.get(cls, {})
        add_line('\n'.join(map(get_new_ml, sorted_dict_values(descrs))))
        add_line(TYPE_INIT(cls))
        
    return lines
        

def auto():
    raise TypeError('use auto2()')
MODULE_DOC = '''
xrand - e(x)tremely fast general use PRNG library for CPython

Implemented entirely in C with micro-optimized code, including
a significant rewrite of Python's abstract object API, xrand gives
Python the ability to compete with compiled languages for developing
software that makes heavy use of random number generation.

The underlying generator will be either xorshift128+ or xoroshiro128+.
By default, 32 bit platforms use xorshift while 64 bit use xoroshiro.
Since the way they are implemented makes speed the only difference
between the two, which one is faster is which one is used.

Except the SystemRandom class (since these generators aren't CS-PRNGS),
xrand has everything the standard library module random.py has and more.
The three new core functions are `flip`, `rand_bytes`, and `shuffled`,
where `flip` is designed to make 50/50 choices super fast, `rand_bytes`
can write its output into a buffer, and `shuffled` efficiently creates
a new, randomized list based on any iterable.

Almost every xrand function is at least one order of magnitude faster
to call than the standard lib version, with several approaching two.
Besides faster functions calls, xrand also supports parallel substreams
guaranteed to non-correlated, which opens up the possibility of using
multiprocessing that doesn't call into question the data's validity.

Most of the new functionality is in the form of a set of new objects
that work conceptually like functools.partial. They are built by passing
the arguments its respective implementation needs to a special
constructor method that builds the object and then instead of calling
the implementation function right away, calls the argument parser that
method uses and keeps the result stored in memory for later use.

That's the new. Here's the different:

LIST OF AFOREMENTIONED OBJECT CONSTRUCTORS:

* choices: returns a `choices` object used with both select and sample
* dice: returns a `dice` object complementing rand_int

All of the following methods return an iterator object: `randiter`

* iter_beta
* iter_expo
* iter_gamma
* iter_lognorm
* iter_norm
* iter_pareto
* iter_triangular
* iter_uniform
* iter_vonmises
* iter_weibull
* flips (not really comparable to `flip`, actually...)

MORE ON THE CORE FUNTIONS NOT IN random.py:

Speaking of `flip`, while it might seem silly or pointless, but when
a simple 50/50 binary choices is needed, nothing else going to come
close to its efficiency.

Second up is `rand_bytes` which is used to fill some object supporting
the buffer protocol with random bytes. When given nothing to fill
it simply returns a new bytes object. Currently it is pretty limited,
but it does work with array.array and possibly some types of np.array.

The final addition is `shuffled`. The difference between it and
`shuffle` is analogous to the difference between the builtin `sorted()`
function and `list.sort`. Like `sorted()`, `shuffled` first unpacks
any iterable into a brand new list but rather than sorting the new list,
it shuffles it. Adding a second way to shuffle allows `shuffle`
to limit itself to work only Python lists, which allows it to remain
around 2 orders of magnitude faster than the shuffle from random.py.

The only functionality xrand doesn't explicitly implement compared
to random.py is the case of using `random.randrange` with multiple
arguments of very large numbers. xrand.select on a range object very
efficiently takes care the multiple argument case when reasonable sized
ints are used, and the new, highly optimized `rand_index` can and should
replace the one argument case.

xrand.choices/select/sample is so radically different from than std lib
version it might as well be considered a new addition as well.
Rather than using THREE separate *sequences* for weighted `choice`s,
A single dict can be used with the keys as members and values as weights.

If for whatever reason (what reason???) the population *must* contain
unhashhables members, don't worry. The `Population` object is a mapping
type that accepts unhashable keys by wrapping them with a `PopEntry` to
guarantee it can be both inserted and retrieved from any Python dict
or set. Also, sampling actually does work with weights.

STUFF WITH DIFFERENT NAMES:

xrand was started with the goal of becoming a drop in replacement for
the ancient random.py, perhaps even one day becoming part of the stdlib.
However as it written before pep8, the names don't cut it for 2018.

In the below table, the call signatures of the random.py functions
are compatible with the xtra version, but not vice versa. For example,
random.randint(a,b) versus xrandom.randint(a, b=0). `choices`, which
is technically not at all the same thing is the sole exception since
it doesn't immediately return a result.


random.py       : xrand
__reduce__      : __getnewargs_ex__
--------------- : __copy__
_randbelow      : rand_index (only difference is it is capped to sys SIZE_MAX)
betavariate     : rv_beta
choice          : select
choices         : select
--------------- : choices (constructor for select/sample complement)
--------------- : dice
expovariate     : rv_expo
--------------- : flip
--------------- : flips
gammavariate    : rv_gamma
gauss           : rv_normal (normalvariate and gauss do the same thing)
getrandits      : rand_bits
getstate        : state / pstate (read only data descriptors)
--------------- : iter_beta
--------------- : iter_expo
--------------- : iter_gamma
--------------- : iter_log_normal
--------------- : iter_normal
--------------- : iter_pareto
--------------- : iter_triangular
--------------- : iter_uniform
--------------- : iter_vonmises
--------------- : iter_weibull
--------------- : jump
logormvariate   : rv_log_normal
normalvariate   : rv_normal
paretovariate   : rv_pareto
--------------- : rand_bytes
randint         : rand_int
randrange       : ---------------
random          : rand_float
sample          : sample
--------------- : split
triangular      : rv_triangular
uniform         : rv_uniform
vonmisesvariate : rv_vonmises
weibullvariate  : rv_weibull


NOTES ON THE UNDERLYING GENERATORS SEED/STATE:

The underlying generator state is structured like this:

* uint64_t state[2]
* uint64_t seed[2]
* size_t pstate
* uint8_t rb

Seed:

The generator's state can optionally be initialized to `seed`. When
provided, the seed must either be an integer or a buffer protocol
supporting object, such as bytes, memoryview, or an array.array.

When an integer seed is provided it must not not be zero or require more
than 128 bits to represent. For buffers, only up to 16 bytes will be
read, and those first 16 bytes similarly must not be all-zero.

Since the underlying generator recovers from a bad seed in less than 8
iterations, (unlike MT19937, which can take 100s of thousands) there's
no reason to modify the input. Since there's only at most 128 bytes to
contend with, reproducibility can be much easier.

When a seed isn't provided, a suitable one is automatically generated
and each interpreter process is guaranteed to never generate the same
seed twice.

State:

Besides floats, which consume 52 of the 60 bits the internal generator
outputs each time it is called, most functions rarely need more than
16, so some of the functions share a small pool of random bits.
Throwing away so many bits doesn't really matter on 64 bit platforms,
but since it is a lot more expensive to generate the next random number
on a 32 bit platforms while costing practically nothing to keep up
with the pool on 64 bit, it is worth the effort..

The main state, which is composed of 2 64 bit unsigned integers, is
viewed with the `state` descriptor.

And now we're at the end, at last:

MISCELLANIOUS STUFF:

'''
