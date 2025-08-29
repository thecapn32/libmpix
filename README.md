libmpix
=======

The library for microcontroller pixel processing.

See the [documentation](https://libmpix.github.io/libmpix/) for how to use it.


JPEGENC
-------

The JPEG encoder implementation comes from Larry Bank at <https://github.com/bitbank2/jpegenc>,
and was temporarily imported into the repo by copying it over to experiment.

An ongoing project is to integrate that library as a submodule instead of copying it,
centralizing the maintenance and improvement into the upstream repository.


Media conversion
----------------

There is an upcoming project to repackage as many functions as possible into a bare-bone library
with extremely low-level API, to allow these basic blocks to be used across several libraries,
each integrating the accelerated low-level implementation their own way.
<https://github.com/sparkfun/micropython-opencv/issues/44>
