
from distutils.core import setup, Extension
from warnings import warn

from settings import *

def assert_defined(macro):
    if macro not in MACROS:
        raise TypeError(f'{macro!r} should not be defined')
def assert_undefined(macro):
    if macro in MACROS:
        raise TypeError(f'{macro!r} should have been defined')
def define(macro, *value):
    MACROS[macro] = (None, *value)[len(value)]

if RAND_AUTO:
    from xrand_auto import *
    if SAVE_RV_AUTOREPR:
        save_autoreprs()
    if SAVE_AUTO_H:
        auto2()

        
if (XOROSHIRO != 'xoroshiro128' or XORSHIFT != 'xorshift128'):
    raise ValueError("core generators renamed")

if RNG_OVERRIDE == XOROSHIRO:
    define('RNG_OVERRIDE', XOROSHIRO)
    define('XSR_GENERATOR')
    
elif RNG_OVERRIDE == XORSHIFT:
    define('RNG_OVERRIDE', XORSHIFT)
    define('XS_GENERATOR')

elif RNG_OVERRIDE is not None:
    raise TypeError(f"invalid generator specified: {FORCED_GENERATOR!r}")


if RAND_TEST_FUNCS:
    define('RAND_TEST_FUNCS')
    
if DEBUG_PARSE_ARGS:
    define('DEBUG_PARSE_ARGS')
    
if not RAND_DEBUG:
    assert_undefined('RAND_DEBUG')
    assert_undefined('RAND_TEST_FUNCS')
    assert_undefined('DEBUG_PARSE_ARGS')
else:
    define('RAND_DEBUG')
    
if INCLUDE_NUMBERSET:
    define('INCLUDE_NUMBERSET')
if INCLUDE_PROFILER:
    define('INCLUDE_PROFILER')

    
DEFINE_MACROS = list(MACROS.items())
assert_defined('_MODULE_')

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



