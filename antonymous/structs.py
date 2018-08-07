'''
After this, basically am now half ways done with a project that will
enable writing classes in a python.py file that will:

* Generate macros that will produce the most efficient C source code
possible for parsing arguments using nothing more than an annotated
Python function signature.

* Able to define in seconds any C typedef that's immediately usable
within Python as a fully featured ctypes subclass. Also all of the
CPython api structs like PyMethodDef, PySequenceMethods, etc will
have be defined as one of these ctypes classes.

* A bunch of other minor stuff in comparison to the most important
thing this thing will accomplish: the 100% automatic generation of
PyTypeObject struct definitions based on a Python class template.

'''
import sys
from functools import lru_cache
from types import MappingProxyType

from singles.attrgetters import *
from singles.checkers import *
from singles.tryexc import *

from publicize import *

ctypes = reimport_module(star_import('ctypes', module=1))
ctypes.__dict__.update(star_import('_ctypes'))

star_import('operator', prefix='op_')
star_import('functools', prefix='_')
PyCStructType = type(Structure)

_getset = type(type.__dict__['__dict__']).__get__

STRUCT_FIELDS  = op_attrgetter('__struct_fields__')
STRUCT_MEMBERS = op_attrgetter('__struct_members__')
STRUCT_BASE    = op_attrgetter('__struct_base__')
CSTRUCT_META   = op_attrgetter('__cstruct_meta__')
CSTRUCT_BASES  = op_attrgetter('__cstruct_bases__')
CSTRUCT_IMPL   = op_attrgetter('__cstruct_impl__')
CSTRUCT        = op_attrgetter('__cstruct__')

@lru_cache()
def _dict_descr(tp):
    return tp.__dict__['__dict__']

def _dict_get(ob):
    tp = type(ob)
    return _dict_descr(tp).__get__(ob, tp)

def _dict_set(ob, value):
    tp = type(ob)
    return _dict_descr(tp).__set__(ob, value)

class struct_fields: # aka annotations

    def __init__(self):
        self.field_names = []

    def __setitem__(self, k, v):
        if hasattr(self, k):
            raise TypeError("duplicate struct member")
        DICT(self).setdefault(k, v)
        self.field_names.append(k)

    @property
    def fields(self):
        d = DICT(self)
        return [(k, d[k]) for k in self.field_names]

    def extend_other(self, other):
        d = DICT(self)
        for field in self.field_names:
            other[field] = d[field]

    __iter__           = property(op_attrgetter('fields.__iter__'))
    __radd__ = __add__ = property(op_attrgetter('fields.__add__'))
    __len__            = property(op_attrgetter('field_names.__len__'))

class struct_members:

    def __init__(self, ns):
        _dict_set(self, ns)

    def __getitem__(self, key):
        d = self.__dict__
        if key in d:
            return d[key]
        if key=='__annotations__':
            return STRUCT_FIELDS(self)
        raise KeyError

    def __setitem__(self, key, value):
        object.__setattr__(self, key, value)

    __delitem__ = object.__delattr__
    pop = property(op_attrgetter('__dict__.pop'))
    keys = property(op_attrgetter('__dict__.keys'))

def prepare_struct(__qualname__, __struct_base__, __struct_bases__):

    @classmethod
    def __cstruct_impl__(cls, name=None, base=None, meta=None, ns=None):

        name       = name or __qualname__
        base       = base or __struct_base__
        ns         = ns or {}
        meta       = meta or __cstruct_meta__

        tp = cls.__cstruct__ = meta(name, __cstruct_bases__, ns)
        cls.__cstruct__ = tp
        tp._fields_ = ([('ob_base', base)] if base else [])+__struct_fields__
        cls.register()
        return tp

    __struct_fields__  = struct_fields()
    __cstruct_meta__   = PyCStructType
    __cstruct_bases__  = (Structure, *__struct_bases__)
    __cstruct__        = None
    __struct_members__ = struct_members(vars())

    return __struct_members__

def check(c):
    n='\n'
    print('struct_fields:', [*STRUCT_FIELDS(c)],n)
    print('cstruct_bases:', CSTRUCT_BASES(c),n)
    print('cstruct:', CSTRUCT(c),n)
    print('cstruct._fields_', CSTRUCT(c)._fields_,n)
    print('class mro:', c.__mro__,n)
    print('metaclass mro:', type(c).__mro__,n)
    print('cstruct mro:', CSTRUCT(c).__mro__, n)

class struct_type(type):

    global STRUCTDEFS, HEADS
    __structs = {}
    __heads = {}
    STRUCTDEFS = MappingProxyType(__structs)
    HEADS = MappingProxyType(__heads)

    @classmethod
    def __prepare__(metacls, name, bases, ob_base=None):
        bname = NAME(bases[0]) if bases else 'struct'
        if bname != 'struct':
            raise TypeError("'struct' must be most derived base, not %r"%bname)
        cbases = [*bases[1:]]
        sbases = []
        while cbases:
            base, *cbases = cbases
            if struct_type_check(base):
                raise TypeError("duplicate 'struct' bases found")
            else:
                sbases.append(base)
        members = prepare_struct(name, ob_base, (*sbases,))
        return members

    def __new__(metacls, name, bases, structure, ob_base=None):
        return type.__new__(metacls, name, bases, DICT(structure))

    def __call__(self, *args):
        return self.__cstruct__(*args)

    @property
    def as_base(structure):
        if STRUCT_FIELDS(structure):
            return [('ob_base', CSTRUCT(structure))]
        return []

    def make_ctype(structure):
        structure.__ctype__ = STRUCTDEFS[NAME(structure)]

    def register(structure):
        name = NAME(structure)
        if name in structure.__structs:
            raise TypeError("duplicate structs with name %r"%name)

        structure.__structs[name] = structure

        def _HEAD():
            new = F_LOCALS(sys._getframe(1))['__struct_fields__']
            assert isinstance(new, struct_fields)
            STRUCT_FIELDS(structure).extend_other(new)

        globals()[f'{name}_HEAD'] = structure.__heads[name] = _HEAD

    __repr__ = property(op_attrgetter('__name__.__str__'))

make_instance_checks(struct_type, ns=globals())

class sb:
    @classmethod
    def _HEAD(old):
        new = F_LOCALS(sys._getframe(1))['__struct_fields__']
        assert isinstance(new, struct_fields)
        STRUCT_FIELDS(old).extend_other(new)

class struct(metaclass=struct_type):

    def __init_subclass__(structure):
        structure.__cstruct_impl__()

class PyObject(struct):
    ob_refcnt: c_ssize_t
    ob_type: c_void_p

class PyVarObject(struct):
    PyObject_HEAD()
    ob_size: c_ssize_t
