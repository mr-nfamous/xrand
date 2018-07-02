
from distutils.core import setup, Extension

# none of this stuff works atm
RAND_DEBUG = False
RAND_AUTO = False
DEBUG_PARSE_ARGS = False
RAND_TEST_FUNCS = False
INCLUDE_NUMBERSET = False
define_macros = []
define = lambda m: define_macros.append((m, None))
extra_compile_args = []


if RAND_DEBUG:
    define('RAND_DEBUG')
if RAND_AUTO:
    from xrand_auto import auto2, save_autoreprs
if RAND_TEST_FUNCS:
    define('RAND_TEST_FUNCS')
    define('INCLUDE_NUMBERSET')

if 1:            
    xrandmodule = Extension('_xrand',
                            ['_xrandmodule.c'],
                            define_macros=define_macros,
                            extra_compile_args=extra_compile_args)
    setup(
        name='xrand',
        version='0.0.3a',
        ext_modules=[xrandmodule],
        py_modules=['xrand'])
