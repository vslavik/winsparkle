[![Build status](https://ci.appveyor.com/api/projects/status/acsuqjheafef29m1?svg=true)](https://ci.appveyor.com/project/vslavik/winsparkle)
[![Crowdin](https://d322cqt584bo4o.cloudfront.net/winsparkle/localized.png)](https://crowdin.com/project/winsparkle)

 About
-------

WinSparkle is a plug-and-forget software update library for Windows
applications. It is heavily inspired by the Sparkle framework for OS X
written by Andy Matuschak and others, to the point of sharing the same 
updates format (appcasts) and having very similar user interface.

See https://winsparkle.org for more information about WinSparkle.

Documentation: [wiki](https://github.com/vslavik/winsparkle/wiki) and
the [winsparkle.h header](https://github.com/vslavik/winsparkle/blob/master/include/winsparkle.h).


 Using prebuilt binaries
-------------------------

The easiest way to use WinSparkle is to download the prebuilt `WinSparkle.dll`
binary. It doesn't have any extra dependencies (not even `msvcr*.dll`) and is
compatible with all Windows compilers.


 Building from sources
-----------------------

If you prefer to build WinSparkle yourself, you can do so.  You'll have to
compile from a git checkout; some of the dependencies are included as git
submodules.

Check the sources out and initialize the submodules:

    $ git clone git://github.com/vslavik/winsparkle.git
    $ cd winsparkle
    $ git submodule init
    $ git submodule update

To compile the library, just open `WinSparkle.sln` (or the one corresponding to
your compiler version) solution and build it.

At the moment, projects for Visual C++ (2008 and up) are provided, so you'll
need that (Express/Community edition suffices). In principle, there's nothing
in the code preventing it from being compiled by other compilers.

There are also unsupported CMake build files in the cmake directory.


 Where can I get some examples?
--------------------------------

Download the sources archive and have a look at the
[examples/](https://github.com/vslavik/winsparkle/tree/master/examples) folder.


 Using latest development versions
-----------------------------------

If you want to stay at the bleeding edge and use the latest, not yet released,
version of WinSparkle, you can get its sources from public repository.
WinSparkle uses git and and the sources are hosted on GitHub at
https://github.com/vslavik/winsparkle

WinSparkle uses submodules for some dependencies, so you have to initialize
them after checking the tree out:

    $ git clone git://github.com/vslavik/winsparkle.git
    $ cd winsparkle
    $ git submodule init
    $ git submodule update

Then compile WinSparkle as described above; no extra steps are required.
