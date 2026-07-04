# Introduction

> **Licenses**  
> Source code: [MIT](https://opensource.org/licenses/MIT).  
> Documentation (this book): [CC BY-NC-ND 4.0](https://creativecommons.org/licenses/by-nc-nd/4.0/) --  
> you may share it with attribution, but may not republish or create derivative works.  

Optimization is an iterative process that requires detailed knowledge of your domain, your target
system, and your toolchain; and a lot of patience. This book teaches the optimization _workflow_
through a concrete example: `rot13`. The algorithm is simple enough to stay out of the way while
still exposing meaningful opportunities to apply profiling tools and low-level techniques.

## `rot13` Refresher

For any character `c`:

- If `c` is a letter: `rot13(c) = base + ((ord(c) - base + 13) % 26)`, where `base` is `ord('a')` for lowercase and `ord('A')` for uppercase.
- Otherwise: `rot13(c) = c`

It is a Caesar cipher with shift 13. The key property that makes it self-inverse is that
`((x + 13) % 26 + 13) % 26 == x`, so encoding and decoding are the same operation.


## What is Optimization

Optimization is the pursuit of improvement; closing the gap between where a system is and where it
could theoretically go. This could be something that runs faster or uses fewer resources. Within each
of these improvements there is a nuance: what is faster, which resource? This is where domain knowledge
becomes mandatory. Defining the theoretically best improvement requires specific insight on the relevant
and irrelevant characteristics of a system. Equally important is knowing when the results are good
enough. Diminishing returns shape most optimization work. Oftentimes, the bulk of an optimization comes
from a single modification leaving only high-effort, less impactful modifications to be found.
