/**
 * Copyright 2016 Leif Walsh
 */

#include <Python.h>

#include <avro.h>

static PyObject *
avro_to_dict(avro_schema_t schema, avro_value_t *value);
static PyObject *
avro_to_dict(avro_schema_t schema, avro_value_t *value) {

#define check(label, x) do {                                            \
        if ((x)) {                                                      \
            PyErr_SetString(PyExc_RuntimeError, avro_strerror());       \
            goto label;                                                 \
        }                                                               \
    } while (0)

    avro_type_t type = avro_value_get_type(value);
    int r;
    switch (type) {
    case AVRO_STRING: {
        const char *str;
        size_t len;
        check(null_exit, avro_value_get_string(value, &str, &len));
        return PyUnicode_FromStringAndSize(str, len-1);
    }
    case AVRO_BYTES: {
        const void *buf;
        size_t len;
        check(null_exit, avro_value_get_bytes(value, &buf, &len));
        return PyBytes_FromStringAndSize((const char *) buf, len);
    }
    case AVRO_FIXED: {
        const void *buf;
        size_t len;
        check(null_exit, avro_value_get_fixed(value, &buf, &len));
        return PyBytes_FromStringAndSize((const char *) buf, len);
    }
    case AVRO_INT32: {
        int32_t i;
        check(null_exit, avro_value_get_int(value, &i));
        return PyLong_FromLong(i);
    }
    case AVRO_INT64: {
        int64_t i;
        check(null_exit, avro_value_get_long(value, &i));
        return PyLong_FromLong(i);
    }
    case AVRO_FLOAT: {
        float f;
        check(null_exit, avro_value_get_float(value, &f));
        return PyFloat_FromDouble(f);
    }
    case AVRO_DOUBLE: {
        double d;
        check(null_exit, avro_value_get_double(value, &d));
        return PyFloat_FromDouble(d);
    }
    case AVRO_BOOLEAN: {
        int i;
        check(null_exit, avro_value_get_boolean(value, &i));
        return PyBool_FromLong(i);
    }
    case AVRO_NULL: {
        Py_RETURN_NONE;
    }
    case AVRO_MAP:
    case AVRO_RECORD: {
        size_t num_fields;
        check(null_exit, avro_value_get_size(value, &num_fields));

        PyObject *dict = PyDict_New();
        if (dict == NULL) {
            goto null_exit;
        }

        const char *name;
        avro_value_t field_val;
        size_t i;
        for (i = 0; i < num_fields; ++i) {
            const avro_schema_t field_schema = (type == AVRO_RECORD
                                                ? avro_schema_record_field_get_by_index(schema, i)
                                                : avro_schema_map_values(schema));
            if (field_schema == NULL) {
                goto decref_dict;
            }
            check(decref_dict, avro_value_get_by_index(value, i, &field_val, &name));
            PyObject *py_field_val = avro_to_dict(field_schema, &field_val);
            if (py_field_val == NULL) {
                goto decref_dict;
            }
            r = PyDict_SetItemString(dict, name, py_field_val);
            Py_DECREF(py_field_val);
            if (r != 0) {
                goto decref_dict;
            }
        }
        return dict;
     decref_dict:
        Py_DECREF(dict);
        goto null_exit;
    }
    case AVRO_ARRAY: {
        size_t num_fields;
        check(null_exit, avro_value_get_size(value, &num_fields));

        avro_schema_t items_schema = avro_schema_array_items(schema);
        if (items_schema == NULL) {
            goto null_exit;
        }

        PyObject *list = PyList_New(num_fields);
        if (list == NULL) {
            goto null_exit;
        }

        avro_value_t field_val;
        size_t i;
        for (i = 0; i < num_fields; ++i) {
            check(decref_list, avro_value_get_by_index(value, i, &field_val, NULL));
            PyObject *py_field_val = avro_to_dict(items_schema, &field_val);
            if (py_field_val == NULL) {
                goto decref_list;
            }
            PyList_SET_ITEM(list, i, py_field_val);
        }
        return list;
     decref_list:
        Py_DECREF(list);
        goto null_exit;
    }
    case AVRO_UNION: {
        int discriminant;
        check(null_exit, avro_value_get_discriminant(value, &discriminant));
        avro_schema_t branch_schema = avro_schema_union_branch(schema, discriminant);
        if (branch_schema == NULL) {
            goto null_exit;
        }
        avro_value_t branch;
        check(null_exit, avro_value_get_current_branch(value, &branch));
        return avro_to_dict(branch_schema, &branch);
    }
    case AVRO_ENUM: {
        int enum_val;
        check(null_exit, avro_value_get_enum(value, &enum_val));
        const char *enum_str = avro_schema_enum_get(schema, enum_val);
        if (enum_str == NULL) {
            goto null_exit;
        }
        return PyUnicode_FromString(enum_str);
    }
    case AVRO_LINK: {
        PyErr_SetString(PyExc_ValueError, "lancaster can't handle links yet");
        goto null_exit;
    }
    }
 null_exit:
    return NULL;

#undef check
}

static inline PyObject *
avro_to_simple_value(avro_schema_t schema, avro_value_t *value) {

#define check(label, x) do {                                            \
        if ((x)) {                                                      \
            PyErr_SetString(PyExc_RuntimeError, avro_strerror());       \
            goto label;                                                 \
        }                                                               \
    } while (0)

    avro_type_t type = avro_value_get_type(value);
    switch (type) {
    case AVRO_STRING: {
        const char *str;
        size_t len;
        check(null_exit, avro_value_get_string(value, &str, &len));
        return PyUnicode_FromStringAndSize(str, len-1);
    }
    case AVRO_BYTES: {
        const void *buf;
        size_t len;
        check(null_exit, avro_value_get_bytes(value, &buf, &len));
        return PyBytes_FromStringAndSize((const char *) buf, len);
    }
    case AVRO_FIXED: {
        const void *buf;
        size_t len;
        check(null_exit, avro_value_get_fixed(value, &buf, &len));
        return PyBytes_FromStringAndSize((const char *) buf, len);
    }
    case AVRO_INT32: {
        int32_t i;
        check(null_exit, avro_value_get_int(value, &i));
        return PyLong_FromLong(i);
    }
    case AVRO_INT64: {
        int64_t i;
        check(null_exit, avro_value_get_long(value, &i));
        return PyLong_FromLong(i);
    }
    case AVRO_FLOAT: {
        float f;
        check(null_exit, avro_value_get_float(value, &f));
        return PyFloat_FromDouble(f);
    }
    case AVRO_DOUBLE: {
        double d;
        check(null_exit, avro_value_get_double(value, &d));
        return PyFloat_FromDouble(d);
    }
    case AVRO_BOOLEAN: {
        int i;
        check(null_exit, avro_value_get_boolean(value, &i));
        return PyBool_FromLong(i);
    }
    case AVRO_NULL: {
        Py_RETURN_NONE;
    }
    case AVRO_ENUM: {
        int enum_val;
        check(null_exit, avro_value_get_enum(value, &enum_val));
        const char *enum_str = avro_schema_enum_get(schema, enum_val);
        if (enum_str == NULL) {
            goto null_exit;
        }
        return PyUnicode_FromString(enum_str);
    }
    case AVRO_UNION:
    case AVRO_MAP:
    case AVRO_RECORD:
    case AVRO_ARRAY:
    case AVRO_LINK: {
        PyErr_SetString(PyExc_ValueError, "when reading to tuple, schema must be a record of simple types");
        goto null_exit;
    }
    }
 null_exit:
    return NULL;

#undef check
}

static PyObject *
avro_to_tuple(avro_schema_t schema, avro_value_t *value) {

#define check(label, x) do {                                            \
        if ((x)) {                                                      \
            PyErr_SetString(PyExc_RuntimeError, avro_strerror());       \
            goto label;                                                 \
        }                                                               \
    } while (0)

    avro_type_t type = avro_value_get_type(value);
    switch (type) {
    case AVRO_RECORD: {
        size_t num_fields;
        check(null_exit, avro_value_get_size(value, &num_fields));

        PyObject *tuple = PyTuple_New(num_fields);
        if (tuple == NULL) {
            goto null_exit;
        }

        const char *name;
        avro_value_t field_val;
        size_t i;
        for (i = 0; i < num_fields; ++i) {
            const avro_schema_t field_schema = avro_schema_record_field_get_by_index(schema, i);
            if (field_schema == NULL) {
                goto decref_tuple;
            }
            check(decref_tuple, avro_value_get_by_index(value, i, &field_val, &name));
            PyObject *py_field_val = avro_to_simple_value(field_schema, &field_val);
            if (py_field_val == NULL) {
                goto decref_tuple;
            }
            PyTuple_SET_ITEM(tuple, i, py_field_val);
        }
        return tuple;
     decref_tuple:
        Py_DECREF(tuple);
        goto null_exit;
    }
    case AVRO_STRING:
    case AVRO_BYTES:
    case AVRO_FIXED:
    case AVRO_INT32:
    case AVRO_INT64:
    case AVRO_FLOAT:
    case AVRO_DOUBLE:
    case AVRO_BOOLEAN:
    case AVRO_NULL:
    case AVRO_MAP:
    case AVRO_ARRAY:
    case AVRO_UNION:
    case AVRO_ENUM:
    case AVRO_LINK: {
        PyErr_SetString(PyExc_ValueError, "when reading to tuple, schema must be a record of simple types");
        goto null_exit;
    }
    }
 null_exit:
    return NULL;

#undef check
}

/**
 * avro_reader_t doesn't give us access to how many bytes it's
 * consumed.  So we sneak around here.  Here are the relevant structs
 * in avro 1.7.7:
 *
 *     enum avro_io_type_t {
 *         AVRO_FILE_IO,
 *         AVRO_MEMORY_IO
 *     };
 *     typedef enum avro_io_type_t avro_io_type_t;
 *
 *     struct avro_reader_t_ {
 *         avro_io_type_t type;
 *         volatile int  refcount;
 *     };
 *
 *     struct _avro_reader_memory_t {
 *         struct avro_reader_t_ reader;
 *         const char *buf;
 *         int64_t len;
 *         int64_t read;
 *     };
 *
 *     typedef struct avro_reader_t_ *avro_reader_t;
 */
static size_t
avro_reader_memory_bytes_read(const avro_reader_t reader) {
    const char *reader_p = (const char *) reader;
    const int64_t *read_p = (const int64_t *)
        (reader_p +
         sizeof(enum { AVRO_FILE_IO, AVRO_MEMORY_IO }) +
         sizeof(int) +
         sizeof(const char *) +
         sizeof(int64_t));
    return *read_p;
}

typedef struct {
    PyObject_HEAD
    avro_schema_t schema;
    avro_value_iface_t *iface;
    avro_value_t value;
} Reader;

static PyObject *
Reader_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Reader *self;

    self = (Reader *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->schema = NULL;
        self->iface = NULL;
    }

    return (PyObject *)self;
}

static int
Reader_init(Reader *self, PyObject *args, PyObject *kwds)
{
    const char *schema_json;
    if (!PyArg_ParseTuple(args, "s", &schema_json)) {
        return -1;
    }

    avro_schema_t schema;
    avro_value_iface_t *iface;

    if (avro_schema_from_json(schema_json, 0, &schema, NULL)) {
        PyErr_SetString(PyExc_ValueError, avro_strerror());
        return -1;
    }

    iface = avro_generic_class_from_schema(schema);
    if (iface == NULL) {
        avro_schema_decref(schema);
        PyErr_SetString(PyExc_RuntimeError, avro_strerror());
        return -1;
    }

    if (avro_generic_value_new(iface, &self->value)) {
        avro_value_iface_decref(iface);
        avro_schema_decref(schema);
        PyErr_SetString(PyExc_RuntimeError, avro_strerror());
        return -1;
    }

    self->schema = schema;
    self->iface = iface;

    return 0;
}

static void
Reader_dealloc(Reader* self)
{
    if (self->schema != NULL) {
        avro_value_decref(&self->value);
        avro_value_iface_decref(self->iface);
        avro_schema_decref(self->schema);
    }
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *
Reader_read_seq(Reader* self, PyObject *args)
{
    const char *buffer;
    int buffer_size;

    PyObject *tuple = NULL;

    if (!PyArg_ParseTuple(args, "y#", &buffer, &buffer_size)) {
        return NULL;
    }

    // Set up avro reader over buffer
    avro_reader_t reader = avro_reader_memory(buffer, buffer_size);
    if (reader == NULL) {
        PyErr_SetString(PyExc_RuntimeError, avro_strerror());
        goto err0;
    }

    // Set up destination list
    PyObject *list = PyList_New(0);
    if (list == NULL) {
        goto err1;
    }

    // Read values out, if the last one is cut off, it returns ENOSPC
    // and we can stop there.  Need to check bytes_read after each
    // successful read, because the last unsuccessful read will
    // advance the pointer and we don't want to count that.
    int r;
    size_t last_bytes_read = 0;
    while ((r = avro_value_read(reader, &self->value)) == 0) {
        last_bytes_read = avro_reader_memory_bytes_read(reader);
        PyObject *py_value = avro_to_dict(self->schema, &self->value);
        if (py_value == NULL) {
            goto err2;
        }
        int r2 = PyList_Append(list, py_value);
        Py_DECREF(py_value);
        if (r2 != 0) {
            goto err2;
        }
    }

    if (r != ENOSPC) {
        PyErr_SetString(PyExc_RuntimeError, avro_strerror());
        goto err2;
    }

    PyObject *py_bytes_read = PyLong_FromLong(last_bytes_read);
    if (py_bytes_read == NULL) {
        goto err2;
    }

    tuple = PyTuple_New(2);
    if (tuple == NULL) {
        Py_DECREF(py_bytes_read);
        goto err2;
    }
    PyTuple_SET_ITEM(tuple, 0, list);
    PyTuple_SET_ITEM(tuple, 1, py_bytes_read);

 err2:
    if (tuple == NULL) {
        // PyTuple_SET_ITEM steals its refcount so if we have a tuple, we don't own list anymore
        Py_DECREF(list);
    }
 err1:
    avro_reader_free(reader);
 err0:
    return tuple;
}

static PyObject *
Reader_read_seq_tuples(Reader* self, PyObject *args)
{
    const char *buffer;
    int buffer_size;

    PyObject *tuple = NULL;

    if (!PyArg_ParseTuple(args, "y#", &buffer, &buffer_size)) {
        return NULL;
    }

    // Set up avro reader over buffer
    avro_reader_t reader = avro_reader_memory(buffer, buffer_size);
    if (reader == NULL) {
        PyErr_SetString(PyExc_RuntimeError, avro_strerror());
        goto err0;
    }

    // Set up destination list
    PyObject *list = PyList_New(0);
    if (list == NULL) {
        goto err1;
    }

    // Read values out, if the last one is cut off, it returns ENOSPC
    // and we can stop there.  Need to check bytes_read after each
    // successful read, because the last unsuccessful read will
    // advance the pointer and we don't want to count that.
    int r;
    size_t last_bytes_read = 0;
    while ((r = avro_value_read(reader, &self->value)) == 0) {
        last_bytes_read = avro_reader_memory_bytes_read(reader);
        PyObject *py_value = avro_to_tuple(self->schema, &self->value);
        if (py_value == NULL) {
            goto err2;
        }
        int r2 = PyList_Append(list, py_value);
        Py_DECREF(py_value);
        if (r2 != 0) {
            goto err2;
        }
    }

    if (r != ENOSPC) {
        PyErr_SetString(PyExc_RuntimeError, avro_strerror());
        goto err2;
    }

    PyObject *py_bytes_read = PyLong_FromLong(last_bytes_read);
    if (py_bytes_read == NULL) {
        goto err2;
    }

    tuple = PyTuple_New(2);
    if (tuple == NULL) {
        Py_DECREF(py_bytes_read);
        goto err2;
    }
    PyTuple_SET_ITEM(tuple, 0, list);
    PyTuple_SET_ITEM(tuple, 1, py_bytes_read);

 err2:
    if (tuple == NULL) {
        // PyTuple_SET_ITEM steals its refcount so if we have a tuple, we don't own list anymore
        Py_DECREF(list);
    }
 err1:
    avro_reader_free(reader);
 err0:
    return tuple;
}

static PyMethodDef Reader_methods[] = {
    {"read_seq", (PyCFunction)Reader_read_seq, METH_VARARGS,
     "Deserialize a sequence of consecutive Avro values.\n"
     "\n"
     ":param schema: json string representing the Avro schema\n"
     ":param buffer: bytes-like object containing binary input\n"
     ":return: seq: a list of python objects, and the number of bytes consumed from buffer\n"},
    {"read_seq_tuples", (PyCFunction)Reader_read_seq_tuples, METH_VARARGS,
     "Deserialize a sequence of consecutive Avro values into tuples.\n"
     "\n"
     "This assumes the input is avro records of simple values (numbers,\n"
     "strings, etc.).\n"
     "\n"
     ":param schema: json string representing the Avro schema\n"
     ":param buffer: bytes-like object containing binary input\n"
     ":return: seq: a list of python tuples, and the number of bytes consumed from buffer\n"},
    {NULL}  /* Sentinel */
};

static PyTypeObject ReaderType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "_lancaster.Reader",        /* tp_name */
    sizeof(Reader),             /* tp_basicsize */
    0,                          /* tp_itemsize */
    (destructor)Reader_dealloc, /* tp_dealloc */
    0,                          /* tp_print */
    0,                          /* tp_getattr */
    0,                          /* tp_setattr */
    0,                          /* tp_reserved */
    0,                          /* tp_repr */
    0,                          /* tp_as_number */
    0,                          /* tp_as_sequence */
    0,                          /* tp_as_mapping */
    0,                          /* tp_hash  */
    0,                          /* tp_call */
    0,                          /* tp_str */
    0,                          /* tp_getattro */
    0,                          /* tp_setattro */
    0,                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE,    /* tp_flags */
    ("A class which can be used to deserialize Avro values from a stream, given a schema.\n"
     "\n"
     ":param str schema: json string representing the Avro schema\n"
     "\n"),                     /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    Reader_methods,             /* tp_methods */
    0,                          /* tp_members */
    0,                          /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    (initproc)Reader_init,      /* tp_init */
    0,                          /* tp_alloc */
    Reader_new,                 /* tp_new */
};

static struct PyModuleDef lancaster = {
   PyModuleDef_HEAD_INIT,
   "_lancaster",   /* name of module */
   NULL, /* module documentation, may be NULL */
   -1,       /* size of per-interpreter state of the module,
                or -1 if the module keeps state in global variables. */
   NULL, NULL, NULL, NULL, NULL
};

PyMODINIT_FUNC
PyInit__lancaster(void)
{
    if (PyType_Ready(&ReaderType) < 0) {
        return NULL;
    }

    PyObject *m = PyModule_Create(&lancaster);
    if (m == NULL) {
        return NULL;
    }
    Py_INCREF(&ReaderType);
    PyModule_AddObject(m, "Reader", (PyObject *)&ReaderType);
    return m;
}
