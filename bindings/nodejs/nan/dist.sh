rm -rf lib/sdb
git clone ../../../ lib/sdb
make -C lib/sdb src/sdb-version.h
####
#gyp --depth=.
#make
