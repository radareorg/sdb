#include <Python.h>
#include <sdb.h>

PyObject * hello(PyObject * self) {
	Sdb *db = sdb_new0();
	sdb_set (db, "foo", "bar", 0);
	sdb_free (db);
	return PyUnicode_FromFormat("Hello C extension!");
}

PyObject * open(PyObject * self, PyObject *args) {
	const char * command = NULL;
	if (!PyArg_ParseTuple(args, "s", &command)) {
		return NULL;
	}
eprintf ("OPEN (%s)\n", command);
	////PyObject *item = PyObject_SetItem(self, file_name);
	PyObject *res = PyUnicode_FromString (command);
	Sdb *db = sdb_new0();
	sdb_set (db, "foo", "bar", 0);
	return res; // PyUnicode_FromFormat("Hello C extension!");
}

char hellofunc_docs[] = "Hello world description.";

PyMethodDef sdb_funcs[] = {
	{	"hello",
		(PyCFunction)hello,
		METH_NOARGS,
		hellofunc_docs},
	{	"open",
		(PyCFunction)open,
		METH_VARARGS,
		hellofunc_docs},
	{	NULL}
};

static const char sdbmod_docs[] = "This is the native sdb module for Python";

PyModuleDef sdb_mod = {
	PyModuleDef_HEAD_INIT,
	"sdb",
	sdbmod_docs,
	-1,
	sdb_funcs,
	NULL,
	NULL,
	NULL,
	NULL
};

PyMODINIT_FUNC PyInit_sdb(void) {
	return PyModule_Create(&sdb_mod);
}
