# ADI2KML

Software to convert [Amateur Data Interchange Format (ADIF)](https://www.adif.org/) to [Keyhole Markup Language (KML)](https://en.wikipedia.org/wiki/Keyhole_Markup_Language).  This allows Amateur Radio operators to view their radio connections (QSOs) inside of KML viewers like [Google Earth](https://earth.google.com).

This software is deployed on the [KO6IUE](https://ko6iue.org/adi2kml) web site so you do not need to install anything to run it.  It will just run inside your browser.

This repository is a place for open-source developers to improve this software over time.

## Getting help

If you tried to use the [online app](https://ko6iue.org/adi2kml) and it failed for you. Please feel free [to file an issue](https://github.com/ko6iue/adi2kml/issues).  

## Performance

This software can convert about 10,000 QSOs a second

```
$ time ./adi2kml sample.adi sample.kml
Processed 1281 QSOs
./adi2kml sample.adi sample.kml  0.12s user 0.00s system 99% cpu 0.126 total
```

Uses around 50 MB of heap per thousand QSOs and is valgrind clean.

```
$ valgrind ./adi2kml sample.adi sample.kml                                        
==2058224== Memcheck, a memory error detector
==2058224== Copyright (C) 2002-2024, and GNU GPL'd, by Julian Seward et al.
==2058224== Using Valgrind-3.26.0 and LibVEX; rerun with -h for copyright info
==2058224== Command: ./adi2kml /home/massie/workspace/adi2kml/sample.adi sample.kml
==2058224== 
Processed 1281 QSOs
==2058224== 
==2058224== HEAP SUMMARY:
==2058224==     in use at exit: 0 bytes in 0 blocks
==2058224==   total heap usage: 761,583 allocs, 761,583 frees, 50,168,648 bytes allocated
==2058224== 
==2058224== All heap blocks were freed -- no leaks are possible
==2058224== 
==2058224== For lists of detected and suppressed errors, rerun with: -s
==2058224== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```
