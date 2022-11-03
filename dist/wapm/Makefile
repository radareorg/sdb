all: sdb.wasm
	vim wapm.toml
	# wapm login
	wapm publish

clean:
	$(MAKE) -C ../.. clean

sdb.wasm:
	$(MAKE) -C ../.. wasi
	cp -f ../../src/sdb.wasm sdb.wasm
