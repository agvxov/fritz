# HIBOT
> IRC bot written in C, responsible for formatting source code.

When hibot receives a direct message,
it will echo it back to the destination channel,
applying simple syntax highlighting rules.

The destination channel is the first channel in `$JOINED`.

### Supported languages:
* Ada
* C/C++
* Fasm

### Bugs:
* Input is line buffered, meaning multi line regions will break
* Since the highlighter was written by Xolatile "The Only Serbian Programmer" Robovic,
and he does not believe in "versioning" nor "finishing software",
hibot uses some anchient version of xyntax which should probably be updated
