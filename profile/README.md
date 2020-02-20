Profiling
---------

You can get timing and callgraph information for a specific *remake* run by using the `--profile` option.
When that is used, the file `callgrind.out`.*pid* is produced and this is in the file format that `valgrind --tool=callgrind` uses.
See the [callgrind format manual](http://valgrind.org/docs/manual/cl-format.html) for some information on this.

Programs that can interpret this are:

* [kcachegrind](http://kcachegrind.sourceforge.net/html/Home.html)
* [gprof2dot](http://github.com/jrfonseca/gprof2dot)
* [callgrind-annotate](http://valgrind.org/docs/manual/cl-manual.html#cl-manual.callgrind_annotate-options)

If there are other programs out there, let me know and I'll add to the list above.

To assist using *graph2dot* to produce a *png* file, I've put together a little shell script to perform a pipe to dot.
