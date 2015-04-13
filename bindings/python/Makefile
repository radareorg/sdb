all:
	valabind --ctypes --module sdb -l sdb ../vala/sdb.vapi

pub publish:
	LC_CTYPE=C python setup.py sdist upload
