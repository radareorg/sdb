#include <Python.h>
#include "../sdbsrc/sdb.h"

PyObject * nsdb_new0(PyObject * self, PyObject *args) {
	void *fd = sdb_new0();
	return PyLong_FromLong((size_t)fd);
}

PyObject * nsdb_now(PyObject * self, PyObject *args) {
	return PyLong_FromLong(sdb_now());
}
PyObject * nsdb_unow(PyObject * self, PyObject *args) {
	return PyLong_FromLong(sdb_unow());
}

PyObject * nsdb_sync(PyObject * self, PyObject *args) {
	void *p = NULL;
	if (!PyArg_ParseTuple(args, "l", &p)) {
		return Py_None;
	}
	sdb_sync (p);
	return Py_None;
}

PyObject * nsdb_add(PyObject * self, PyObject *args) {
	const char * k = NULL;
	const char * v = NULL;
	void *p = NULL;
	if (!PyArg_ParseTuple(args, "lss", &p, &k, &v)) {
		return Py_None;
	}
	int res = sdb_add (p, k, v, 0);
	return PyBool_FromLong (res);
}

PyObject * nsdb_set(PyObject * self, PyObject *args) {
	const char * k = NULL;
	const char * v = NULL;
	void *p = NULL;
	if (!PyArg_ParseTuple(args, "lss", &p, &k, &v)) {
		return Py_None;
	}
	sdb_set (p, k, v, 0);
	return Py_None;
}

PyObject * nsdb_get(PyObject * self, PyObject *args) {
	const char * k = NULL;
	void *p = NULL;
	if (!PyArg_ParseTuple(args, "ls", &p, &k)) {
		return Py_None;
	}
	char *a = sdb_get (p, k, 0);
	if (!a) {
		return Py_None;
	}
	PyObject * res = PyUnicode_FromFormat(a);
	free (a);
	return res;
}

PyObject * nsdb_query(PyObject * self, PyObject *args) {
	void *db = NULL;
	const char *q = NULL;
	if (!PyArg_ParseTuple(args, "ls", &db, &q)) {
		return NULL;
	}
	char *s = sdb_querys (db, NULL, 0, q);
	PyObject * res = PyUnicode_FromFormat(s);
	return res;
}

PyObject * nsdb_open(PyObject * self, PyObject *args) {
	void *db = NULL;
	const char * path = NULL;
	if (!PyArg_ParseTuple(args, "ls", &db, &path)) {
		return NULL;
	}
	int fd = sdb_open (db, path);	
	PyObject *res = PyLong_FromLong(fd);
	return res;
}

PyObject * pysdb_free(PyObject * self, PyObject *args) {
	void *db = NULL;
	if (!PyArg_ParseTuple (args, "l", &db)) {
		return NULL;
	}
	sdb_free (db);
	return Py_None;
}

const char hellofunc_docs[] = "Hello world description.";

PyMethodDef sdb_funcs[] = {
	{	"now",
		(PyCFunction)nsdb_now,
		METH_NOARGS,
		hellofunc_docs},
	{	"unow",
		(PyCFunction)nsdb_unow,
		METH_NOARGS,
		hellofunc_docs},
	{	"sync",
		(PyCFunction)nsdb_sync,
		METH_VARARGS,
		hellofunc_docs},
	{	"new0",
		(PyCFunction)nsdb_new0,
		METH_NOARGS,
		hellofunc_docs},
	{	"get",
		(PyCFunction)nsdb_get,
		METH_VARARGS,
		hellofunc_docs},
	{	"add",
		(PyCFunction)nsdb_add,
		METH_VARARGS,
		hellofunc_docs},
	{	"set",
		(PyCFunction)nsdb_set,
		METH_VARARGS,
		hellofunc_docs},
	{	"query",
		(PyCFunction)nsdb_query,
		METH_VARARGS,
		hellofunc_docs},
	{	"open",
		(PyCFunction)nsdb_open,
		METH_VARARGS,
		hellofunc_docs},
	{	"free",
		(PyCFunction)free,
		METH_VARARGS,
		hellofunc_docs},
	{	NULL}
};

static const char sdbmod_docs[] = "This is the native sdb module for Python";

PyModuleDef sdb_mod = {
	PyModuleDef_HEAD_INIT,
	"nsdb",
	sdbmod_docs,
	-1,
	sdb_funcs,
	NULL,
	NULL,
	NULL,
	NULL
};

PyMODINIT_FUNC PyInit_nsdb(void) {
	return PyModule_Create(&sdb_mod);
}
