all:
	$(MAKE) -C ../../..
	cp -f binding.gyp.dist binding.gyp
	npm install
	node test.js

bin:
	-git clone ../../.. sdb
	cd sdb ; git pull
	cp -f binding.gyp.nogit binding.gyp
	$(MAKE) -C sdb CFLAGS="-fPIC -O3"
	npm install

clean:
	rm -rf build npm-debug.log node_modules
