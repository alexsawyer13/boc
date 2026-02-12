# boc (Builder of C)

A build system for C, written in C, configured in C. No more Visual Studio, Make, CMake, gcc, clang, blah blah blah. Just C.

## Summary

boc_init is a program you install. It is a command line utility for using boc. Running boc_init in a directory copies a template boc.c to your directory and compiles it. Running ./boc will RECOMPILE boc if it has changed, and then run itself again if necessary. This means that you never need to use a compiler at all. boc_init will start you off with ./boc and it will recompile itself. boc.c will then contain your entire build system. On windows, there is no place for default .h files, so boc.h will be copied into your directory too. This isn't necessary (boc could just include boc.h with every compilation), but it is useful because then you don't have to faff around with things like vscode, it will just detect boc.h as you would hope for it to.

## Idea
- boc_init.c is compiled and installed (through package manager or installer, this will all built using boc itself!)
- Once boc_init is installed, ./boc_init will initialise a repository and compile boc.c for the first time
- Then we bootstrap endlessly!
- boc has a "command", "flags" and "parameters"
- ./boc command -flag1 param1 param2 -flag2 param1 -flag3 param1 param2
- By default ./boc corresponds to ./boc build which is the default command