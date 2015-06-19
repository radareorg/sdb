sdb-json
========

Fast JSON library for NodeJS based on sdb's js0n implementation.

This module offers an API to parse, indent and manipulate JSON
using the SDB library which results in better performance than
native v8 one.

The `encode` and `decode` methods are faster base64 enc/dec (2-3x)

Usage
-----

This is a sample program using sdb-json:

	var json = require('sdb-json');
	var msg = '{"foo":123,"bar":[1,2,3,4,6]}'

	/* json indentation */
	console.log (json.indent (msg));
	console.log (json.unindent (json.indent (msg)));

	/* base64 */
	console.log ("Hello "+json.encode ("World"));
	console.log ("Hello "+json.decode ("V29ybGQ="));

	/* manipulating json */
	json.set(msg, "bar[2]", "pop");
	console.log (json.get(msg, "bar[2]"));

Benchmarks
----------
The `genjson.js` script is used to create random JSON documents.

	$ node genjson.js <size> <depth> > a.json

It turns out that v8 parses JSON recursively and fails to load
documents with thousands of nested elements.

Benchmark for 68K JSON file
---------------------------

	$ node test.js 
	[BENCHMARK] - lower numbers and higher ratios are better
	[T] base64 1000 times...
	 >> sdb : 226
	 >> v8 : 607
	 ratio : 2.6
	[T] get 1000 times...
	 >> sdb : 241
	 >> v8 : 595
	 ratio : 2.4
	[T] set 1000 times...
	 >> sdb : 241
	 >> v8 : 599
	 ratio : 2.4
	[T] indent 1000 times...
	 >> sdb : 305
	 >> v8 : 22640
	 ratio : 74.2
	[T] indent2 1000 times...
	 >> sdb : 309
	 >> v8 : 593
	 ratio : 1.9

Benchmark for 4MB JSON file
---------------------------

	[BENCHMARK] - lower numbers and higher ratios are better
	[T] base64 500000 times...
	 >> sdb : 199
	 >> v8 : 313
	 ratio : 1.5
	[T] get 500000 times...
	 >> sdb : 510
	 >> v8 : 354
	 ratio : 0.6
	[T] set 500000 times...
	 >> sdb : 518
	 >> v8 : 354
	 ratio : 0.6
	[T] indent 500000 times...
	 >> sdb : 229
	 >> v8 : 1316
	 ratio : 5.7
	[T] indent2 500000 times...
	 >> sdb : 221
	 >> v8 : 292
	 ratio : 1.3

