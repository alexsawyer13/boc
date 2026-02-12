set -e

gcc boc_init.c -o boc_init
cp boc.h /usr/local/include/
mv boc_init /usr/local/bin