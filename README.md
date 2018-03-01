# BMP Super Resolution

[![license](https://img.shields.io/github/license/mashape/apistatus.svg?maxAge=2592000)](http://opensource.org/licenses/MIT)
![build](https://img.shields.io/badge/build-success-brightgreen.svg)
![language](https://img.shields.io/badge/language-C-green.svg)

A simple test for CNN based BMP super resolution

100% C language

## Effect

For test, I use a picture from pixiv by
[(ˉ﹃ˉ)EM1234](https://www.pixiv.net/member.php?id=8467971)

This is a effect preview:

![preview](./in_vs_out.png)

The left is origin and the right is preview after two times convert (Resolution 4 times up!)


I don't supposes to use it to do anything, because **it is VERY VERY slow**, the preview before cost me 9 hours on a Intel Core i7-4558U.

## TODO

I'd like to change it from CPU caculate from GPU use openCL to speed it up
