# To use boc on your system you must use boc_init to set up the repository.
# boc_init is actually built using boc itself! But we don't distribute any binaries.
# Therefore, you must compile boc.c manually the first time before it can bootstrap.
# Once you've done that, running ./boc will recompile boc_init.
# You can then use install.sh to put it in your /usr/local/ directory, so boc_init can be used anywhere!

set -e # Exit on error
gcc -o boc boc.c # Compile boc, NOTE(Alex S): IF THIS EVER GETS CHANGED, REMEMBER TO CHANGE _boc_compile_boc_c