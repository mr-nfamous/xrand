
===============================================
``xrand``: Extremely fast Cpython PRNG lib
===============================================

Implemented entirely in C with micro-optimized code, including
a significant rewrite of Python's abstract object API, xrand gives
Python the ability to compete with compiled languages for developing
software which makes heavy use of random number generation.

Includes all features from the standard library module ``random.py`` and more.

Some comparisons
-----------------
Here's a quick script comparing the difference in function calls per second xrand is capable of versus the equivalent random.py function. (ran on Ivy Bridge Core i7 on Windows 32-bit Python 3.6.0)

.. -code-begin-

.. code-block:: pycon

    import random
    from functools import partial
    from itertools import product, repeat, starmap

    import xrand
    from tabulate import tabulate
    from xrand import *

    def compare(f, xrv, pyv, *args, xargs=None, rep='', **kws):
        xargs = kws.pop('xargs', args)
        x = perf(getattr(xrand, xrv), xargs)
        y = perf(getattr(random, pyv), args)
        z = x/y-1.0
        data.append([f'{f}({rep})', x, y, z])

    cards = [f'{a}{b}' for a,b in product('HSDC',(*'23456789','10',*'JKQA'))]
    loaded_dice = choices({1:1, 2:1, 3:1, 4:1, 5:1, 6:5})
    py_pop, py_weights = loaded_dice.population
    py_dice = partial(random.choices,py_pop, cum_weights=py_weights)
    data = [('f', 'xrand', 'random', 'dif')]

    a = perf(iter_beta(None, 3.0, 3.0).take)
    b = perf(random.betavariate, (3.0, 3.0))
    data.append(['betavariate(3.0, 3.0)', a, b, a/b])
    compare('choice', 'select', 'choice', cards, rep='cards')
    a = perf(loaded_dice.next_n, (10,))
    b = perf(py_dice, (), {'k':10})
    data.append(['choices(cards, k=10)', a, b, a/b])
    compare('shuffle', 'shuffle', 'shuffle', cards, rep='cards')
    compare('sample', 'sample', 'sample', cards, 20, rep='cards, 20')
    compare('randint', 'rand_int', 'randint', 0, 385, xargs=(385,), rep='385') 
    compare('random', 'rand_float', 'random')
    compare('randrange', 'rand_index', 'randrange', 76000, rep='76000')
    for i, (a, b, c, d) in enumerate(data[1:], 1):
        data[i] = [a, f'{round(b):_}', f'{round(c):_}', f'{d:+,.0%}']

    >>> print(tabulate(data))
    ---------------------  ----------  ----------  -------
    f                      xrand       random      dif
    betavariate(3.0, 3.0)  4_181_041   163_915     +2,551%
    choice(cards)          18_632_245  759_560     +2,353%
    choices(cards, k=10)   2_618_083   126_883     +2,063%
    shuffle(cards)         1_480_818   16_220      +9,029%
    sample(cards, 20)      1_329_424   30_537      +4,253%
    randint(385)           11_644_439  455_809     +2,455%
    random()               33_648_747  27_141_605  +24%
    randrange(76000)       14_391_397  588_787     +2,344%
    ---------------------  ----------  ----------  -------
