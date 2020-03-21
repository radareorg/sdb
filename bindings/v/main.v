module main

import sdb

pub fn main() {
	s := sdb.new0()
	s.set('hello', 'world')
	// s.reset()
	s.add('hello', 'error')
	println('stats ${s.str()}')
	name := s.get('hello')
	println('hello ${name}')
	s.free()
}
