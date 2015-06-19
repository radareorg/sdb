/* sdb-json example */

var json = require('bindings')('sdb-json');
var fs = require("fs");

var smalljson = true;


if (smalljson) {
	var msg = '{"foo":33,"bar":[1,2,3,4,5]}';
} else {
	//var msg = ''+fs.readFileSync('a.json');
	var msg = ''+fs.readFileSync('c.json');
}
/*
   console.log(msg);
   console.log(json.indent(msg));
   console.log(json.unindent(json.indent(msg)));

   console.log(json.get(msg, "bar[2]"));
 */

console.log("[BENCHMARK] - lower numbers and higher ratios are better");
if (smalljson) {
	run(1, 500000);
} else {
	run(1, 2);
	//run(1, 1000);
}

function benchmark(name, cb) {
	var a = new Date();
	cb ();
	var b = new Date();
	var delta = b-a;
	console.log (" >> "+name+" : "+delta);
	return delta;
}

function delta(a,b) {
	var c = a/b;
	c *= 10;
	c |= 0;
	c /= 10;
	return c;
}

function run_base64 (times) {
	var sdb = benchmark("sdb", function() {
		var ret = 0;
		for (var i=0; i<times; i++) {
			ret += json.encode(msg).length;
		}
	});

	try {
		var v8 = benchmark("v8", function() {
			var obj = JSON.parse(msg);
			var ret = 0;
			for (var i=0; i<times; i++) {
				ret += JSON.stringify(obj).length;
			}
		});
	} catch (e) {
		console.error (" >> v8", e);
		v8 = 1;
	}
	return delta(v8, sdb);
}

function run_set(times) {
	var sdb = benchmark("sdb", function() {
		for (var i=0; i<times; i++) {
			json.set(msg, "bar[2]", "pop");
		}
	});

	try {
		var v8 = benchmark("v8", function() {
			for (var i=0; i<times; i++) {
				var obj = JSON.parse(msg);
				obj["bar"][2] = "pop";
			}
		});
	} catch (e) {
		console.error (" >> v8", e);
		v8 = 1;
	}
	return delta(v8, sdb);
}

function run_get(times) {
	var sdb = benchmark("sdb", function() {
		var ret = 0;
		for (var i=0; i<times; i++) {
			var fun = json.get(msg, "bar[2]");
			ret += +fun;
		}
	});

	try {
		var v8 = benchmark("v8", function() {
			var ret = 0;
			for (var i=0; i<times; i++) {
				var fun = JSON.parse(msg)["bar"][2];
				ret += +fun;
			}
		});
	} catch (e) {
		console.error (" >> v8", e);
		v8 = 1;
	}

	return delta(v8, sdb);
}

function run_indent(times) {
	var sdb = benchmark("sdb", function() {
		for (var i=0; i<times; i++) {
			json.indent(msg);
		}
	});

	try {
		var v8 = benchmark("v8", function() {
			var obj = JSON.parse(msg);
			for (var i=0; i<times; i++) {
				//JSON.stringify(obj, null, "    "); // 6x
				//JSON.stringify(obj, null, 4); // 7x
				JSON.stringify(obj, null, 1); // 7x
			}
		});
	} catch (e) {
		console.error (" >> v8", e);
		v8 = 1;
	}

	return delta(v8, sdb);
}

function run_indent2(times) {
	var sdb = benchmark("sdb", function() {
		for (var i=0; i<times; i++) {
			json.indent(msg);
		}
	});
	try {
		var v8 = benchmark("v8", function() {
			var obj = JSON.parse(msg);
			for (var i=0; i<times; i++) {
				JSON.stringify(obj);
			}
		});
	} catch (e) {
		console.error (" >> v8 ", e);
		v8 = 1;
	}
	return delta(v8, sdb);
}

function runBenchmark (name, cb, limit, times) {
	console.log ("[T] "+name+" "+times+" times...");
	for (var i = 0; i<limit; i++) {
		console.log (" ratio : "+ cb(times));
		times <<= 2;
	}
}

function run(limit, times) {
	runBenchmark("base64", run_base64, limit, times);
	runBenchmark("get", run_get, limit, times);
	runBenchmark("set", run_set, limit, times);
	runBenchmark("indent", run_indent, limit, times);
	runBenchmark("indent2", run_indent2, limit, times);
}

