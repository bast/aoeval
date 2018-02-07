[![Build Status](https://travis-ci.org/bast/balboa.svg?branch=master)](https://travis-ci.org/bast/balboa/builds)
[![License](https://img.shields.io/badge/license-%20MPL--v2.0-blue.svg)](../master/LICENSE)

# balboa

Balboa computes Gaussian basis functions and their geometric derivatives for a
batch of points in space.

```
"You know all there is to know about fighting, so there's no sense us going down
that same old road again. To beat this guy, you need speed - you don't have it.
And your knees can't take the pounding, so hard running is out. And you got
arthritis in your neck, and you've got calcium deposits on most of your joints,
so sparring is out.  So, what we'll be calling on is good ol' fashion blunt
force trauma. Horsepower. Heavy-duty, cast-iron, piledriving punches that will
have to hurt so much they'll rattle his ancestors. Every time you hit him with
a shot, it's gotta feel like he tried kissing the express train. Yeah! Let's
start building some hurtin' bombs!" [Rocky Balboa]
```


### Status

Experimental code.
Under heavy development.
Nothing is stable.


### Requirements and dependencies

You need the following to install the code:

- C++ compiler
- [CMake](https://cmake.org)
- [Python](https://www.python.org)

For testing you need:

- [CFFI](https://cffi.readthedocs.io)
- [Pytest](http://doc.pytest.org)
- [Numpy](http://www.numpy.org)


### Installation and testing

```
virtualenv venv
source venv/bin/activate
pip install -r requirements.txt
cmake -H. -Bbuild
cd build
cmake --build .
ctest
```


### Ordering of AOs

We use he following naming:

```
geo_000: undifferentiated
geo_100: 1st-order derivative wrt x
geo_010: 1st-order derivative wrt y
...

```

For N basis functions and P points the ordering is given by:

```
[ geo_000                            ][ geo_100 ][ geo_010 ][ geo_001 ][ geo_200 ] ...
[ ao_1    ][ ao_2    ] ... [ ao_N    ]
[ 1 ... P ][ 1 ... P ] ... [ 1 ... P ]
```
