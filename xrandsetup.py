
from distutils.core import setup, Extension
from warnings import warn

from settings import *

if RAND_AUTO:
    from xrand_auto import *
    if SAVE_RV_AUTOREPR:
        save_autoreprs()
    if SAVE_AUTO_H:
        auto2()
if (XOROSHIRO != 'xoroshiro128' or XORSHIFT != 'xorshift128'):
    raise ValueError("core generators renamed")
if RNG_OVERRIDE == XOROSHIRO:
    MACROS['RNG_OVERRIDE'] = XOROSHIRO
    MACROS['XSR_GENERATOR'] = None
elif RNG_OVERRIDE == XORSHIFT:
    MACROS['RNG_OVERRIDE'] = XORSHIFT
    MACROS['XS_GENERATOR'] = None
    
elif RNG_OVERRIDE:
    raise TypeError(f"invalid generator specified: {FORCED_GENERATOR!r}")

_debug_mode = RAND_DEBUG is not None
_opt = None
_fail = False
if RAND_TEST_FUNCS:
    _opt = MACROS['RAND_TEST_FUNCS'] = None
if DEBUG_PARSE_ARGS:
    _opt = MACROS['DEBUG_PARSE_ARGS'] = None
if not _debug_mode:
    if _opt:
        raise TypeError("cannot use debug opts without defining RAND_DEBUG")
else:
    MACROS['RAND_DEBUG'] = None
if INCLUDE_NUMBERSET:
    MACROS['INCLUDE_NUMBERSET'] = None
if INCLUDE_PROFILER:
    MACROS['INCLUDE_PROFILER'] = None
DEFINE_MACROS = list(MACROS.items())
assert '_MODULE_' in MACROS
assert 'RAND_DEBUG' in MACROS
if 1:            
    xrandmodule = Extension(Ext_MODULE,
                            ['_xrandmodule.c'],
                            define_macros=DEFINE_MACROS,
                            extra_compile_args=COMPILE_ARGS)
    setup(
        name=Py_MODULE,
        version='0.0.3a',
        ext_modules=[xrandmodule],
        py_modules=[Py_MODULE])



