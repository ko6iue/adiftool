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
