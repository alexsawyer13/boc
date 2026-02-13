# To use boc on your system, your repository needs a boc.c in it with build commands.
# It also needs to be compiled manually for the first time!
# Once compiled manually once, running ./boc will detect changes in boc.c and recompile itself
# boc_init is a program which will copy a template boc.c file and do that first compilation step
# Simply run boc_init while in your directory, and ./boc.c and ./boc will be created!
# This script copies ./boc_init and ./boc.h from this repo into /usr/local/ so they're accessible anywhere

set -e # Exit on error

# Run ./boc to compile boc_init.c
./boc
# Copy files to the correct place
sudo cp boc.h /usr/local/include/
sudo cp boc_init /usr/local/bin/
sudo cp boc_template.c /usr/local/src/
echo "Installed boc successfully"