# BMP Super Resolution

[![license](https://img.shields.io/github/license/mashape/apistatus.svg?maxAge=2592000)](http://opensource.org/licenses/MIT)
![language](https://img.shields.io/badge/language-C-green.svg)
![pipeline status](https://www.short-circuits.org/git/yeonji/Image-Super-Resolution/badges/develop/pipeline.svg)

A simple test for CNN based BMP super resolution

Main program 100% C language

## Build Program

### Mac OS X command line

You need to install a command line compiler.

As I have only tried Xcode Command Line Tools, I suggest you to use it too.

Run `xcode-select --install` to install them.

To compile it, you need to run:

```
make dirs
make LDFLAGS=-framework\ OpenCL
```

## License

### MIT License

Copyright (c) 2018-2019 Yeonji@ieee.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
