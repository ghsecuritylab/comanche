#ifndef __TYPES_H__
#define __TYPES_H__

#include <common/logging.h>
#include <api/kvstore_itf.h>
#include <Python.h>
#include <structmember.h>
#include "pool_type.h"

extern PyTypeObject PoolType;

static PyObject * pool_close(Pool* self);
static PyObject * pool_count(Pool* self);
static PyObject * pool_put(Pool* self, PyObject *args, PyObject *kwds);
static PyObject * pool_get(Pool* self, PyObject *args, PyObject *kwds);



static PyObject *
Pool_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  auto self = (Pool *)type->tp_alloc(type, 0);
  assert(self);
  return (PyObject*)self;
}

Pool * Pool_new()
{
  return (Pool *) PyType_GenericAlloc(&PoolType,1);
}

/** 
 * tp_dealloc: Called when reference count is 0
 * 
 * @param self 
 */
static void
Pool_dealloc(Pool *self)
{
  assert(self);
  Py_TYPE(self)->tp_free((PyObject*)self);
  PLOG("Pool: dealloc");
}

static PyMemberDef Pool_members[] = {
  //  {"port", T_ULONG, offsetof(Pool, _port), READONLY, "Port"},
  {NULL}
};

PyDoc_STRVAR(put_doc,"Pool.put(key,value) -> Write key-value pair to pool.");
PyDoc_STRVAR(get_doc,"Pool.get(key) -> Read value from pool.");
PyDoc_STRVAR(close_doc,"Pool.close() -> Forces pool closure. Otherwise close happens on deletion.");
PyDoc_STRVAR(count_doc,"Pool.count() -> Get number of objects in the pool.");

static PyMethodDef Pool_methods[] = {
  {"close",(PyCFunction) pool_close, METH_NOARGS, close_doc},
  {"count",(PyCFunction) pool_count, METH_NOARGS, count_doc},
  {"put",  (PyCFunction) pool_put, METH_VARARGS | METH_KEYWORDS, put_doc},
  {"get",  (PyCFunction) pool_get, METH_VARARGS | METH_KEYWORDS, get_doc},
  {NULL}
};



PyTypeObject PoolType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "dawn.Pool",           /* tp_name */
  sizeof(Pool)   ,      /* tp_basicsize */
  0,                       /* tp_itemsize */
  (destructor) Pool_dealloc,      /* tp_dealloc */
  0,                       /* tp_print */
  0,                       /* tp_getattr */
  0,                       /* tp_setattr */
  0,                       /* tp_reserved */
  0,                       /* tp_repr */
  0,                       /* tp_as_number */
  0,                       /* tp_as_sequence */
  0,                       /* tp_as_mapping */
  0,                       /* tp_hash */
  0,                       /* tp_call */
  0,                       /* tp_str */
  0,                       /* tp_getattro */
  0,                       /* tp_setattro */
  0,                       /* tp_as_buffer */
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
  "Pool",              /* tp_doc */
  0,                       /* tp_traverse */
  0,                       /* tp_clear */
  0,                       /* tp_richcompare */
  0,                       /* tp_weaklistoffset */
  0,                       /* tp_iter */
  0,                       /* tp_iternext */
  Pool_methods,         /* tp_methods */
  Pool_members,         /* tp_members */
  0,                       /* tp_getset */
  0,                       /* tp_base */
  0,                       /* tp_dict */
  0,                       /* tp_descr_get */
  0,                       /* tp_descr_set */
  0,                       /* tp_dictoffset */
  0, //(initproc)Pool_init,  /* tp_init */
  0,            /* tp_alloc */
  Pool_new,             /* tp_new */
  0, /* tp_free */
};

  

static PyObject * pool_put(Pool* self, PyObject *args, PyObject *kwds)
{
  static const char *kwlist[] = {"key",
                                 "value",
                                 "no_stomp",
                                 NULL};

  const char * key = nullptr;
  PyObject * value = nullptr;
  int do_not_stomp = 0;
  
  if (! PyArg_ParseTupleAndKeywords(args,
                                    kwds,
                                    "sO|p",
                                    const_cast<char**>(kwlist),
                                    &key,
                                    &value,
                                    &do_not_stomp)) {
    PyErr_SetString(PyExc_RuntimeError,"bad arguments");
    return NULL;
  }

  assert(self->_kvstore);
  assert(self->_pool != IKVStore::POOL_ERROR);

  if(self->_pool == 0) {
    PyErr_SetString(PyExc_RuntimeError,"already closed");
    return NULL;
  }

  void * p = nullptr;
  size_t p_len = 0;
  if(PyByteArray_Check(value)) {
    p = PyByteArray_AsString(value);
    p_len = PyByteArray_Size(value);
  }
  else if(PyUnicode_Check(value)) {
    p = PyUnicode_DATA(value);
    p_len = PyUnicode_GET_DATA_SIZE(value);
  }

  unsigned int flags = 0;
  auto hr = self->_dawn->put(self->_pool,
                             key,
                             p,
                             p_len,
                             flags);
                                    
  if(hr != S_OK) {
    std::stringstream ss;
    ss << "pool.put failed [status:" << hr << "]";
    PyErr_SetString(PyExc_RuntimeError,ss.str().c_str());
    return NULL;
  }
                                    
  Py_INCREF(self);
  return (PyObject *) self;
}

static PyObject * pool_get(Pool* self, PyObject *args, PyObject *kwds)
{
  static const char *kwlist[] = {"key",
                                 NULL};

  const char * key = nullptr;
  
  if (! PyArg_ParseTupleAndKeywords(args,
                                    kwds,
                                    "s",
                                    const_cast<char**>(kwlist),
                                    &key)) {
    PyErr_SetString(PyExc_RuntimeError,"bad arguments");
    return NULL;
  }

  assert(self->_kvstore);
  assert(self->_pool != IKVStore::POOL_ERROR);

  if(self->_pool == 0) {
    PyErr_SetString(PyExc_RuntimeError,"already closed");
    return NULL;
  }

  void * out_p = nullptr;
  size_t out_p_len = 0;
  auto hr = self->_dawn->get(self->_pool,
                             key,
                             out_p,
                             out_p_len);
  
  if(hr != S_OK || out_p == nullptr) {
    std::stringstream ss;
    ss << "pool.get failed [status:" << hr << "]";
    PyErr_SetString(PyExc_RuntimeError,ss.str().c_str());
    return NULL;
  }

  /* copy value string */
  auto result = PyUnicode_FromString(static_cast<const char*>(out_p));
  self->_dawn->free_memory(out_p);

  return result;
}


static PyObject * pool_close(Pool* self)
{
  
  assert(self->_dawn);
  assert(self->_pool != IKVStore::POOL_ERROR);

  if(self->_pool == 0) {
    PyErr_SetString(PyExc_RuntimeError,"Dawn.Pool.close failed. Already closed.");
    return NULL;
  }

  status_t hr = self->_dawn->close_pool(self->_pool);
  self->_pool = 0;

  if(hr != S_OK) {
    std::stringstream ss;
    ss << "pool.close failed [status:" << hr << "]";
    PyErr_SetString(PyExc_RuntimeError,ss.str().c_str());
    return NULL;
  }
  
  PLOG("dawn::pool_close OK!");
  Py_INCREF(self);
  return (PyObject *) self;
}

static PyObject * pool_count(Pool* self)
{
  assert(self->_dawn);
  assert(self->_pool != IKVStore::POOL_ERROR);

  if(self->_pool == 0) {
    PyErr_SetString(PyExc_RuntimeError,"Dawn.Pool.count failed. Already closed.");
    return NULL;
  }

  return PyLong_FromSize_t(self->_dawn->count(self->_pool));
}


#endif