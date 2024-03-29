# this file has been automatically generated by valabind
import sys
from ctypes import *
from ctypes.util import find_library
if sys.platform.startswith('win'):
	lib = WinDLL (find_library ('sdb'))
else:
	lib = CDLL (find_library ('sdb'))
def rlist2array(x,y):
	it = x.iterator ()
	ret = []
	while True:
		data = it.get_data ()
		ds = cast (data, POINTER(y)).contents
		ret.append (ds)
		if it.n == None:
			break
		it = it.get_next ()
	return ret

class AddressHolder(object):
	def __get__(self, obj, type_):
		if getattr(obj, '_address', None) is None:
			obj._address = addressof(obj)
		return obj._address

	def __set__(self, obj, value):
		obj._address = value

class WrappedRMethod(object):
	def __init__(self, cname, args, ret):
		self.cname = cname
		self.args = args
		self.ret = ret
		self.args_set = False
		self.method = getattr(lib, cname)

	def __call__(self, *a):
		if not self.args_set:
			if self.args:
				self.method.argtypes = [eval(x.strip()) for x in self.args.split(',')]
			self.method.restype = eval(self.ret) if self.ret else None
			self.args_set = True
		b = []
		for i in range(0, len(a)):
			t = self.method.argtypes[i]
			if issubclass(t, c_char_p):
				b.append(create_string_buffer(bytes(a[i], 'utf-8')))
			else:
				b.append(a[i])
		if issubclass(self.method.restype, c_char_p):
			return str(bytes(self.method(*b)))
		return self.method(*b)

class WrappedApiMethod(object):
	def __init__(self, method, ret2, last):
		self.method = method
		self._o = None
		self.ret2 = ret2
		self.last = last

	def __call__(self, *a):
		result = self.method(self._o, *a)
		print(self.ret2)
		if self.ret2 == 'c_char_p':
			result = eval(self.ret2)(bytes(result, 'utf-8')) # "PENE" #// str(bytes(eval(self.ret2)(result), 'utf-8'))
		elif self.ret2:
			result = eval(self.ret2)(result)
		if self.last:
			return getattr(result, self.last)
		return result

	def __get__(self, obj, type_):
		self._o = obj._o
		return self

def register(cname, args, ret):
	ret2 = last = None
	if ret:
		if ret[0]>='A' and ret[0]<='Z':
			x = ret.find('<')
			if x != -1:
				ret = ret[0:x]
			last = 'contents'
			ret = 'POINTER('+ret+')'
		else:
			last = 'value'
			ret2 = ret
			
	method = WrappedRMethod(cname, args, ret)
	wrapped_method = WrappedApiMethod(method, ret2, last)
	return wrapped_method, method

class SignalSource(Structure): #0
	_fields_ = [
	]
	def __init__(self, signum):
		Structure.__init__(self)
		g_unix_signal_source_new = lib.g_unix_signal_source_new
		g_unix_signal_source_new.restype = c_void_p
		self._o = g_unix_signal_source_new (signum)

	_o = AddressHolder()


class Sdb(Structure): #1
	_fields_ = [
	]
	def __init__(self, path, file, locked):
		Structure.__init__(self)
		sdb_new = lib.sdb_new
		sdb_new.restype = c_void_p
		self._o = sdb_new (path, file, locked)

	_o = AddressHolder()

	ns, sdb_ns = register('sdb_ns','c_void_p, c_char_p, c_bool','Sdb')
	ns_path, sdb_ns_path = register('sdb_ns_path','c_void_p, c_char_p, c_bool','Sdb')
	sync, sdb_sync = register('sdb_sync','c_void_p','c_bool')
	query, sdb_query = register('sdb_query','c_void_p, c_char_p','c_bool')
	querys, sdb_querys = register('sdb_querys','c_void_p, c_char_p, c_int, c_char_p','c_char_p')
	exists, sdb_exists = register('sdb_exists','c_void_p, c_char_p','c_bool')
	get, sdb_get = register('sdb_get','c_void_p, c_char_p, POINTER(c_uint)','c_char_p')
	add, sdb_add = register('sdb_add','c_void_p, c_char_p, c_char_p, c_uint','c_bool')
	set, sdb_set = register('sdb_set','c_void_p, c_char_p, c_char_p, c_uint','c_bool')
	bool_get, sdb_bool_get = register('sdb_bool_get','c_void_p, c_char_p, POINTER(c_uint)','c_bool')
	bool_set, sdb_bool_set = register('sdb_bool_set','c_void_p, c_char_p, c_bool, c_uint','c_bool')
	array_length, sdb_array_length = register('sdb_array_length','c_void_p, c_char_p','c_int')
	array_get, sdb_array_get = register('sdb_array_get','c_void_p, c_char_p, c_int, POINTER(c_uint)','c_char_p')
	array_set, sdb_array_set = register('sdb_array_set','c_void_p, c_char_p, c_int, c_char_p, c_uint','c_bool')
	array_set_num, sdb_array_set_num = register('sdb_array_set_num','c_void_p, c_char_p, c_int, c_ulonglong, c_uint','c_bool')
	array_delete, sdb_array_delete = register('sdb_array_delete','c_void_p, c_char_p, c_int, c_uint','c_bool')
	array_remove, sdb_array_remove = register('sdb_array_remove','c_void_p, c_char_p, c_char_p, c_uint','c_bool')
	array_remove_num, sdb_array_remove_num = register('sdb_array_remove_num','c_void_p, c_char_p, c_ulonglong, c_uint','c_bool')
	array_contains, sdb_array_contains = register('sdb_array_contains','c_void_p, c_char_p, c_char_p, POINTER(c_uint)','c_bool')
	num_exists, sdb_num_exists = register('sdb_num_exists','c_void_p, c_char_p','c_bool')
	num_set, sdb_num_set = register('sdb_num_set','c_void_p, c_char_p, c_ulonglong, c_uint','c_bool')
	num_get, sdb_num_get = register('sdb_num_get','c_void_p, c_char_p, POINTER(c_uint)','c_ulonglong')
	num_inc, sdb_num_inc = register('sdb_num_inc','c_void_p, c_char_p, c_ulonglong, c_uint','c_ulonglong')
	num_dec, sdb_num_dec = register('sdb_num_dec','c_void_p, c_char_p, c_ulonglong, c_uint','c_ulonglong')
	json_get, sdb_json_get = register('sdb_json_get','c_void_p, c_char_p, c_char_p, POINTER(c_uint)','c_char_p')
	json_set, sdb_json_set = register('sdb_json_set','c_void_p, c_char_p, c_char_p, c_char_p, c_uint','c_bool')
	json_num_get, sdb_json_num_get = register('sdb_json_num_get','c_void_p, c_char_p, c_char_p, POINTER(c_uint)','c_int')
	json_num_set, sdb_json_num_set = register('sdb_json_num_set','c_void_p, c_char_p, c_char_p, c_int, c_uint','c_int')
	json_num_inc, sdb_json_num_inc = register('sdb_json_num_inc','c_void_p, c_char_p, c_char_p, c_int, c_uint','c_int')
	json_num_dec, sdb_json_num_dec = register('sdb_json_num_dec','c_void_p, c_char_p, c_char_p, c_int, c_uint','c_int')
	json_indent, sdb_json_indent = register('sdb_json_indent','c_char_p','c_char_p')
	json_unindent, sdb_json_unindent = register('sdb_json_unindent','c_char_p','c_char_p')
	unset, sdb_unset = register('sdb_unset','c_void_p, c_char_p, c_int','c_bool')
	reset, sdb_reset = register('sdb_reset','c_void_p',None)
	unlink, sdb_unlink = register('sdb_unlink','c_void_p',None)
	expire_get, sdb_expire_get = register('sdb_expire_get','c_void_p, c_char_p, POINTER(c_uint)','c_ulonglong')
	expire_set, sdb_expire_set = register('sdb_expire_set','c_void_p, c_char_p, c_ulonglong, c_uint','c_bool')
	now, sdb_now = register('sdb_now','','c_ulonglong')

