# xrand
xrand - e(x)tremely fast general purpose PRNG library for CPython

Do people actually read readmes?

Only ever been compiled for 3.6 using MSVC on Windows

I hate setup.py and want everything to exist within the extension module, but
wanted this online the minute I was certain everything works flawlessly.

Lots of direct memory manipulation for integer conversions that I've slowly tried to phase out but
I'm sure it definitely has still something in it that will break big endian systems 
(if there even is such a thing)

This is the 3rd complete refactor that begun mid april 2018 and ended on july 2018.

Desperately needs yet another, now that everything is functioning simultaneously.

There's about 5000 lines of tests and scripts that build half of this thing automagically
that are not currently in the repo, but nobody else could read them but me ,
and the thought of anyone else seeing them is like one of those dreams where you go to
school and realize out you're not wearing any pants.
