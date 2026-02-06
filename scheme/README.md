# adi2kml Scheme code

**This directory is not part of the main build. It is being
shared for any schemers who want to see the original Scheme
code. This project has been rewritten in C in order to
use WASM.  I tried to use Guile Hoot for WASM but ran into
too many limitations/bugs.**

Scheme scripts for reading an Amateur Data Interchange Format (ADIF) file
and writing the data into Keyhole Markup Language (KML) format.

Most loggers and sites like [QRZ](https://www.qrz.com/) support ADIF file export.

## Running to build your own KML file

You will need to install [CHICKEN scheme](https://www.call-cc.org/). `adi2kml` only uses modules shipped with CHICKEN Scheme so no further setup is needed.

CHICKEN scheme runs on [a number of different platforms](https://wiki.call-cc.org/portability) so `adi2kml` should run on your favorite (or semi-favorite) OS. I've only tested it on Linux.

Clone this github repo by running:

```
$ git clone https://github.com/ko6iue/adi2kml.git
$ cd ./adi2kml/scheme
```

You can run two ways: scheme script or compiled C binary.

To run the script, use a commandline like the following:

```
$ ./adi2kml.scm input.adi output.kml
```

assuming your QTH maidenhead grid is `JK42GO`, your ADIF file is named `input.adi`, and you want to export all the QSOs into `output.kml`.

To compile the scheme scripts into a C binary, run the following command

```
$ csc -o adi2kml adi2kml.scm
```

If you want to copy this binary to another machine which doesn't have CHICKEN scheme installed, you will need to use static linking with the `-static` flag, e.g.

```
$ csc -static -o adi2kml adi2kml.scm
```


You use the binary, just like the script, running e.g.

```
$ ./adi2kml input.adi output.kml
```


## Performance

The scheme script is able to process about 4600 QSOs per second.

```
time ./adi2kml.scm sample.adi sample.kml
Processed 1281 unique callsigns with maidenhead data

real    0m0.274s
user    0m0.264s
sys     0m0.010s
```

The compiled binary is able to process about 5900 QSOs per second (a 28% improvement).

```
time ./adi2kml sample.adi sample.kml
Processed 1281 unique callsigns with maidenhead data

real    0m0.217s
user    0m0.207s
sys     0m0.009s
```

This was run on my laptop with an Intel i9-14900HX (32) @ 5.600GHz processor.

Both the script and binary version of the program use very little memory. The `adi2kml` program is designed to stream the ADIF directly to KML with as little intermediate data as possible.

## Maidenhead Calculations

The `adi2kml.scm` script loads the `maidenhead.scm` file in order to have methods for converting maidenhead coordinates to GPS and calculating bearing and distance between two maidenhead locators.

All GPS coordinates, bearing, and distances are calculated and do not rely on that information being present in the ADIF file aside from the keys `my_gridsquare` and `gridsquare`.

## Limitations and TODOs

* The ADIF parser I wrote uses simple regular expressions. It works for ADIF files exported by [QRZ](https://qrz.com) and [QLog](https://github.com/foldynl/QLog) but your mileage may vary. There is currently no support for the ADX File Format.
* Only the first QSO for each callsign is processed. The others are ignored; otherwise, a person you talk with often will have multiple pins at their location.  This could be handled better in the future.
* I just started writing tests. More will follow with time.
* ADIF records without a maidenhead grid are ignored for now.
