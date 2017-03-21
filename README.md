# dukgyp

[gyp.js][1] on a [duktape][2].

## Description

[gyp.js][1] is a JavaScript port of [GYP][3]. While it is really fast and slick
when run on node.js, it is impossible to build node.js on a new platform with
just `gyp.js`.

`dukgyp` solves bootstrapping problem by providing very low-level C bindings to
polyfill `fs` and `child_process` APIs and using `duktape` as a JS runtime.

## Build Instructions

This project can be built on system without node.js, but to get the source file
one need to build it somewhere else where node.js is available:

```sh
npm install
npm run build
```

dukgyp can be tested with:
```sh
npm test
```

The result of the build is copied to `bin` folder:
```sh
./bin/dukgyp
```

The single-source file is placed in:
```sh
./out/Release/dukgyp-bundle.c
```

Source file may be compiled with any compiler without any flags or defines.
Simple!

## How to contribute?

See [issues][0] with `help-wanted` tag.

## LICENSE

This software is licensed under the MIT License.

Copyright Fedor Indutny, 2017.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to permit
persons to whom the Software is furnished to do so, subject to the
following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
USE OR OTHER DEALINGS IN THE SOFTWARE.


### Duktape license

(http://opensource.org/licenses/MIT)

Copyright (c) 2013-2017 by Duktape authors (see AUTHORS.rst)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

[0]: https://github.com/indutny/dukgyp/issues?q=is%3Aissue+is%3Aopen+label%3A%22help+wanted%22
[1]: http://github.com/indutny/gyp.js
[2]: http://duktape.org/
[3]: https://gyp.gsrc.io/
[4]: https://github.com/indutny/dukgyp/issues/5
