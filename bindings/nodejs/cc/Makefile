all:
	@if [ -d build ]; then node-waf ; else node-waf configure ; node-waf ; fi
	cp build/Release/sdb.node .
	node test.js

clean:
	rm -f sdb.node
	node-waf clean

mrproper: clean
	rm -rf build
