# dtrans Distance Transform utility

```
Copyright (C) 2010 Roland Philippsen. All rights reserved.
Released under new BSD license. See LICENSE.BSD for details.
```


## Building the Library and Examples

You need [git][], GNU [Make][], and [PNG development][] files. If you have [FLTK][], you can enable the graphical example by editing the Makefile before building. Maybe there will at some point be downloadable tarballs, so you won't need [git][], and a configure script, so you won't need to edit a Makefile.

    $ git clone git://github.com/poftwaresatent/dtrans.git
    $ cd dtrans
    $ make

[git]: http://git-scm.com/
[Make]: http://www.gnu.org/software/make/
[PNG development]: http://www.libpng.org/pub/png/libpng.html
[FLTK]: http://www.fltk.org/


## Building and Testing the Python Extension Module

You need [Python][] development files and `distutils`. This has been tested with Python-2.5 on OS X, so your mileage may vary:

    $ python setup.py build
    $ PYTHONPATH=build/lib.macosx-10.6-i386-2.5 python
    Python 2.5.5 (r255:77872, Jul 29 2010, 20:37:57)
    [GCC 4.2.1 (Apple Inc. build 5659)] on darwin
    Type "help", "copyright", "credits" or "license" for more
    information.
    >>> import dtrans
    >>> dt=dtrans.DistanceTransform(400, 400, 0.1)
    >>> dt.setDist(0, 0, 0)
    True
    >>> dt.compute()
    >>> dt.getDist(10, 20)
    2.3110537930644446 

[Python]: http://www.python.org/


## Building the Documentation

You need [Doxygen][] for this.

    $ doxygen Doxyfile

[Doxygen]: http://www.doxygen.org/

And then open the `html/index.html` file in a web browser.


## Running the Examples

The `test` program is just a very bare-bones sanity check. It is more intended as a source example to get you started than an actual (unit) test:

    $ ./test

The `pngdtrans` application on the other hand is supposed to be useful right away. It allows you to use 8-bit grayscale PNG files as input and output to the distance transform, and it comes with some built-in help:

    $ ./pngdtrans -h

For example, if you want to plan a path through a maze, you need a speed map file (in this case `maze.png` adapted from [this one][http://en.wikipedia.org/wiki/File:Maze01-01.png]) and a file which specifies your goal location (in this case `goal.png`):

    $ ./pngdtrans -s maze.png -i goal.png -o path.png

Now open the `path.png` file. It is a grayscale image which encodes the distance to the goal, taking into account the obstacles, of each point in the environment. This allows you to find the path from any non-obstacle point to the goal, by following the negative gradient of this distance map. In other words, if you start at some pixel location, always go in the direction of its darkest neighbor, and you will eventually end up at the lower-right exit of the maze.

This grayscale image of the distance transform is not necessarily the easiest output format for controlling e.g. a robot's motion. It is better to use the library version of `dtrans` and rely on the `dtrans::DistanceTransform::computeGradient()` method.

There also is a stub of a graphical example, which gets built if you have [FLTK][] and edit the Makefile accordingly. Right now it does not do much, just compute the distance transform in an empty square environment and display the gradient directions:

    $ ./gdtrans

Maybe by the time you read this it'll be possible to read PNG input files into gdtrans, and to trace robot paths from any point in the environment. Or maybe you feel like implementing that and sending me a patch?
