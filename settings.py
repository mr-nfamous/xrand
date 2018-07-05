
# xrand options: modify these constants before compiling
# most of this probably doesn't work atm (if any does at all)


Ext_MODULE = 'xrand'
Py_MODULE = 'brand' # test module to avoid name conflicts
PROFILE_LOOPS = 25_000_000
# debug mode: nothing works unless RAND_DEBUG is True
RAND_DEBUG        = False
RAND_TEST_FUNCS   = False
DEBUG_PARSE_ARGS  = False

# auto generate source code: RAND_AUTO must be True
RAND_AUTO         = None
SAVE_RV_AUTOREPR  = False # builds tp_repr for iter_rv funcs
SAVE_AUTO_H       = False # builds auto.h

# without FORCE_GENERATOR the generator chosen depends on platform
XOROSHIRO         = 'xoroshiro128'
XORSHIFT          = 'xorshift128'
RNG_OVERRIDE      = None # must be XOROSHIRO or XORSHIFT

# extras
INCLUDE_NUMBERSET = False #include a lightly wrapped version of internal set
INCLUDE_PROFILER = True #include "perf" function

# is a module level function and should never actually be used
INCLUDE_SETSTATE = False #enable setstate func

COMPILE_ARGS = []
MACROS = {'_MODULE_': Ext_MODULE,
          'PROFILE_LOOPS': str(PROFILE_LOOPS)}
