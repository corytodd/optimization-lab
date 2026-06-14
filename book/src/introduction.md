# Introduction

> **Licenses**  
> Source code: [MIT](https://opensource.org/licenses/MIT).  
> Documentation (this book): [CC BY-NC-ND 4.0](https://creativecommons.org/licenses/by-nc-nd/4.0/) --  
> you may share it with attribution, but may not republish or create derivative works.  

Optimization is an iterative process that requires detailed knowledge of your domain, your target
system, and your toolchain; and a lot of patience. This book teaches the _workflow_ through a
concrete example: `rot13`. The algorithm is simple enough to stay out of the way while still
exposing meaningful opportunities to apply profiling tools and low-level techniques.

## `rot13` Refresher

For any character `c`:

- If `c` is a letter: `rot13(c) = base + ((ord(c) - base + 13) % 26)`, where `base` is `ord('a')` for lowercase and `ord('A')` for uppercase.
- Otherwise: `rot13(c) = c`

It is a Caesar cipher with shift 13. The key property that makes it self-inverse is that
`((x + 13) % 26 + 13) % 26 == x`, so encoding and decoding are the same operation.
