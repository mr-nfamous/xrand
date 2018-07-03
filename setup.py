
from distutils.core import setup, Extension
from warnings import warn

# xrand options: modify these constants before compiling
# most of this probably doesn't work atm (if any does at all)

Ext_MODULE = 'xrand'
Py_MODULE = 'brand' # test module to avoid name conflicts

# debug mode: nothing works unless RAND_DEBUG is True
RAND_DEBUG        = None
RAND_TEST_FUNCS   = False
DEBUG_PARSE_ARGS  = False

# auto generate source code: RAND_AUTO must be True
RAND_AUTO         = None
SAVE_RV_AUTOREPR  = False # builds tp_repr for iter_rv funcs
SAVE_AUTO_H       = False # builds auto.h

# without FORCE_GENERATOR the generator chosen depends on platform
# will break if used right now
XOROSHIRO         = 'xoroshiro128'
XORSHIFT          = 'xorshift128'
FORCED_GENERATOR  = None # must be XOROSHIRO or XORSHIFT
INCLUDE_NUMBERSET = False

COMPILE_ARGS = []
MACROS = {'_MODULE_': Ext_MODULE}



if RAND_AUTO:
    from xrand_auto import *
    if SAVE_RV_AUTOREPR:
        save_autoreprs()
    if SAVE_AUTO_H:
        auto2()

_forced = None
if FORCED_GENERATOR == XOROSHIRO:
    _forced = MACROS['RNG_OVERRIDE'] = XOROSHIRO
elif FORCED_GENERATOR == XORSHIFT:
    _forced = MACROS['RNG_OVERRIDE'] = XORSHIFT
elif FORCED_GENERATOR:
    raise TypeError(f"invalid generator specified: {FORCED_GENERATOR!r}")

_debug_mode = RAND_DEBUG is not None
_opt = None
_fail = False
if RAND_TEST_FUNCS:
    _opt = MACROS['RAND_TEST_FUNCS'] = None
if DEBUG_PARSE_ARGS:
    _opt = MACROS['DEBUG_PARSE_ARGS'] = None
if _opt and not _debug_mode:
    raise TypeError("cannot use debug opts without defining RAND_DEBUG")

MACROS = list(MACROS.items())

if 1:            
    xrandmodule = Extension(Ext_MODULE,
                            ['_xrandmodule.c'],
                            define_macros=MACROS,
                            extra_compile_args=COMPILE_ARGS)
    setup(
        name=Py_MODULE,
        version='0.0.3a',
        ext_modules=[xrandmodule],
        py_modules=[Py_MODULE])



