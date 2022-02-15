all: sdb.wasm
	vim wapm.toml
	# wapm login
	wapm publish

sdb.wasm:
	$(MAKE) -C ../.. wasi
	cp -f ../../src/sdb.wasm sdb.wasm
