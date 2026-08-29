# Introduction

> **Licenses**  
> Source code: [MIT](https://opensource.org/licenses/MIT).  
> Documentation (this book): [CC BY-NC-ND 4.0](https://creativecommons.org/licenses/by-nc-nd/4.0/) --  
> you may share it with attribution, but may not republish or create derivative works.  

Optimization is an iterative process that requires detailed knowledge of your
domain, your target system, and your toolchain; and a lot of patience. This book
teaches the optimization _workflow_ through a concrete example: `rot13`. The
algorithm is simple enough to stay out of the way while still exposing
meaningful opportunities to apply profiling tools and low-level techniques.

After Chapter 01, each section is intended to be self-standing. Remember to have
fun and take your time working through the material.

## `rot13` Refresher

Encode letters (a-z, A-Z) in a string by shifting 13 letters right. Decode
letters by shifting 13 letters to the left. Non-letters are a nop and case does
not change. Wrapping before `a` or after `z`
returns to `z` or `a`, respectively.

```
Hello World! It's 2026. => Uryyb Jbeyq! Vg'f 2026.
```

More precisely, the ROT13 cipher transformation is defined by the following
piecewise function:

\\[
R(c) = \begin{cases} B_c + \Big(\big(\text{ord}(c) - B_c + 13\big) \bmod 26\Big), & \text{if } c \text{ is an alphabetic character} \\\\ c, & \text{otherwise} \end{cases}
\\]

Where the base offset \\(B_c \in \{\text{ord('a')}, \text{ord('A')}\}\\) is a
constant determined by the case of the character:

\\[
B_c = \begin{cases} \text{ord('a')}, & \text{if } c \text{ is lowercase} \\\\ \text{ord('A')}, & \text{if } c \text{ is uppercase} \end{cases}
\\]


A defining property of this configuration is that the transformation is an
involution (a self-inverse function), meaning \\(E(E(x)) = x\\). We prove this
algebraic identity below:

\\[
\begin{aligned}
E(E(x)) &\equiv (E(x) + 13) \pmod{26} \\\\
&\equiv ((x + 13) + 13) \pmod{26} \\\\
&\equiv (x + 26) \pmod{26} \\\\
&\equiv x \pmod{26}
\end{aligned}
\\]

Because \\(26 \equiv 0 \pmod{26}\\), the shift composition simplifies perfectly.
Consequently, the decryption function \\(D(x)\\) is identical to the encryption
function:

\\[
D(x) = E(x)
\\]



## What is Optimization

Optimization is the pursuit of improvement. The science of closing the gap
between where a system is and where it could theoretically go. This could be
something that runs faster or uses fewer resources. Within each of these
improvements there is a nuance: what does faster mean, which resource? This is
where domain knowledge becomes mandatory. Defining the theoretically best
improvement requires specific insight on the relevant and irrelevant
characteristics of a system. Equally important is knowing when the results are
good enough.

It's also possible that your outcome is the conclusion that system is already
optimal. Working through the process of proving how your code spends it
resources is valuable because you're left with the tools and data required to
monitor performance for the life of your system. Optimization is an act of
learning and nurturing, both for yourself and your system.
