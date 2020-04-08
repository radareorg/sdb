function randString() {
	return Math.random().toString(30).replace(/[^a-z]+/g, '');
}

function randInteger() {
	return (Math.random()*10000) | 0;
}

function randFloat() {
	return Math.random()*1000;
}

function randBoolean() {
	return (Math.random()*100)>50? true: false;
}

function randArray(rec, depth) {
	if (depth<1) return null;
	var arr = [];
	for (var i =0; i<rec; i++) {
		do {
			var o = randType(3, depth-1);
		} while (typeof o == 'object');
		arr.push (o);
	}
	return arr;
}

function randType(rec, depth) {
if (depth<1)return null;
	var options = [
		randString,
		randInteger,
		randFloat,
		randBoolean,
		randObject,
	];
	var idx = (Math.random()*100)|0;
	idx %= Object.keys(options).length;
	return options[idx](rec, depth-1);
}

function randObject(rec, depth) {
	if (depth<1) return null;
	var obj = {};
	for (let i=0; i<rec; i++) {
		var k = randString();
		obj[k] = randType(rec, depth-1);
	}
	obj["bar"] = [1,2,3,4,5,6,7];
	return obj;
}

var argv = process.argv.splice(1);
switch (argv.length()) {
case 1: var obj = randObject(+argv[0], 156); break;
case 2: var obj = randObject(+argv[0], +argv[1]); break;
default: var obj = randObject(1500, 156); break;
}
console.log(JSON.stringify (obj));
