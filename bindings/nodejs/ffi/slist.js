
var Sdb = require ("./");

function SList(name) {
	const pfx = 'slist.'+name+'.';
	var db = Sdb.open('', '');
	this.add = function (data) {
		var old = db.get (pfx+'first');
		var idx = db.num_inc (pfx+'last');
		db.set(pfx+'first', ''+idx);
		db.set(pfx+idx+'.next', ''+idx);
		db.set(pfx+idx, db.encode(data));
		db.set(pfx+idx+'.next', old);
		return idx;
	}
	this.del = function (idx) {
		var nxt = db.get(pfx+idx+'.next');
		db.set(pfx+idx, db.get(pfx+nxt))
		db.set(pfx+idx+'.next', db.get(pfx+nxt+'.next'))
		db.unset(pfx+nxt);
		db.unset(pfx+nxt+'.next');
	}
	this.forEach = function(cb) {
		var idx = db.get(pfx+'first');
		var data = db.get(pfx+idx);
		while (data) {
			cb(db.decode(data));
			if (!(idx = db.get(pfx+idx+'.next')))
				break;
			data = db.get(pfx+idx);
		}
	}
	this.dump = function () {
		db.query('*');
	}
}

var s = new SList('s');
var rm = s.add ('destroy');
s.add ('world');
s.add ('hello');
s.del (rm);
s.dump();
s.forEach(function(x) {
	console.log ('==>',x);
});



