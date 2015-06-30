rm -rf sdb
git clone ../../../ sdb
make -C sdb src/sdb-version.h
####
#gyp --depth=.
#make
