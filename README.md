# ADI2KML

Software to convert [Amateur Data Interchange Format (ADIF)](https://www.adif.org/) to [Keyhole Markup Language (KML)](https://en.wikipedia.org/wiki/Keyhole_Markup_Language).  This allows Amateur Radio operators to view their radio connections (QSOs) inside of KML viewers like [Google Earth](https://earth.google.com).

This software is deployed on the [KO6IUE](https://ko6iue.org/adi2kml) web site so you do not need to install anything to run it.  It will just run inside your browser.

This repository is a place for open-source developers to improve this software over time.

## Getting help

If you tried to use the [online app](https://ko6iue.org/adi2kml) and it failed for you. Please feel free [to file an issue](https://github.com/ko6iue/adi2kml/issues).  

## Performance

This software can convert over 120,000 QSOs a second

```
$ time ./adi2kml -i sample.adi -o sample.kml
Processed 8670 QSOs
./adi2kml sample.adi sample.kml  0.07s user 0.01s system 98% cpu 0.080 total
```

Uses around 1.5 MB of heap per thousand QSOs and is valgrind clean.

```
valgrind ./adi2kml sample.adi sample.kml
==81186== Memcheck, a memory error detector
==81186== Copyright (C) 2002-2024, and GNU GPL'd, by Julian Seward et al.
==81186== Using Valgrind-3.26.0 and LibVEX; rerun with -h for copyright info
==81186== Command: ./adi2kml all.adi foo.kml
==81186== 
Processed 8670 QSOs
==81186== 
==81186== HEAP SUMMARY:
==81186==     in use at exit: 78,785 bytes in 11,159 blocks
==81186==   total heap usage: 34,135 allocs, 22,976 frees, 13,811,506 bytes allocated
==81186== 
==81186== LEAK SUMMARY:
==81186==    definitely lost: 78,785 bytes in 11,159 blocks
==81186==    indirectly lost: 0 bytes in 0 blocks
==81186==      possibly lost: 0 bytes in 0 blocks
==81186==    still reachable: 0 bytes in 0 blocks
==81186==         suppressed: 0 bytes in 0 blocks
==81186== Rerun with --leak-check=full to see details of leaked memory
==81186== 
==81186== For lists of detected and suppressed errors, rerun with: -s
==81186== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)

```
