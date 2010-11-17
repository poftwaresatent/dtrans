/*
 * Copyright (c) 2010 Roland Philippsen <roland DOT philippsen AT gmx DOT net>
 *
 * BSD license:
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of
 *    contributors to this software may be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR THE CONTRIBUTORS TO THIS SOFTWARE BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <Python.h>
#include "structmember.h"

#include "DistanceTransform.hpp"
#include <sstream>


typedef struct {
    PyObject_HEAD
    dtrans::DistanceTransform * dt;
} dtrans_object;


static void
dtrans_dealloc(dtrans_object * self)
{
  delete self->dt;
  self->ob_type->tp_free((PyObject*)self);
}


static PyObject *
dtrans_new(PyTypeObject * type, PyObject * args, PyObject * kwds)
{
  dtrans_object * self;
  
  self = (dtrans_object *) type->tp_alloc(type, 0);
  if (self != NULL) {
    self->dt = 0;
  }
  
  return (PyObject *) self;
}


static int
dtrans_init(dtrans_object * self, PyObject * args, PyObject * kwds)
{
  unsigned int dimx, dimy;
  double scale;
  if ( ! PyArg_ParseTuple(args, "IId", &dimx, &dimy, &scale)) {
    return -1;
  }

  delete self->dt;		// redundant?
  self->dt = new dtrans::DistanceTransform(dimx, dimy, scale);
  
  return 0;
}


static PyMemberDef dtrans_members[] = {
  {NULL}
};


static PyObject *
dtrans_foo(dtrans_object * self)
{
  std::ostringstream msg;
  msg << self->dt->dimX() << " "
      << self->dt->dimY() << " "
      << self->dt->scale();
  
  return PyString_FromString(msg.str().c_str());
}


static PyMethodDef dtrans_methods[] = {
  {"foo", (PyCFunction) dtrans_foo, METH_NOARGS,
   "Return a string combining the dimensions and the scale."
  },
  {NULL}  /* Sentinel */
};


static PyTypeObject dtrans_type = {
  PyObject_HEAD_INIT(NULL)
  0,                                        /* ob_size*/
  "dtrans.DistanceTransform",               /* tp_name*/
  sizeof(dtrans_object),                    /* tp_basicsize*/
  0,                                        /* tp_itemsize*/
  (destructor) dtrans_dealloc,              /* tp_dealloc*/
  0,                                        /* tp_print*/
  0,                                        /* tp_getattr*/
  0,                                        /* tp_setattr*/
  0,                                        /* tp_compare*/
  0,                                        /* tp_repr*/
  0,                                        /* tp_as_number*/
  0,                                        /* tp_as_sequence*/
  0,                                        /* tp_as_mapping*/
  0,                                        /* tp_hash */
  0,                                        /* tp_call*/
  0,                                        /* tp_str*/
  0,                                        /* tp_getattro*/
  0,                                        /* tp_setattro*/
  0,                                        /* tp_as_buffer*/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags*/
  "DistanceTransform type",                 /* tp_doc */
  0,                                        /* tp_traverse */
  0,                                        /* tp_clear */
  0,                                        /* tp_richcompare */
  0,                                        /* tp_weaklistoffset */
  0,                                        /* tp_iter */
  0,                                        /* tp_iternext */
  dtrans_methods,                           /* tp_methods */
  dtrans_members,                           /* tp_members */
  0,                                        /* tp_getset */
  0,                                        /* tp_base */
  0,                                        /* tp_dict */
  0,                                        /* tp_descr_get */
  0,                                        /* tp_descr_set */
  0,                                        /* tp_dictoffset */
  (initproc) dtrans_init,                   /* tp_init */
  0,                                        /* tp_alloc */
  dtrans_new,                               /* tp_new */
};


static PyMethodDef module_methods[] = {
  {NULL}  /* Sentinel */
};


PyMODINIT_FUNC
initdtrans(void)
{
  PyObject * module;
  
  dtrans_type.tp_new = PyType_GenericNew;
  if (PyType_Ready(&dtrans_type) < 0) {
    return;
  }
  
  module = Py_InitModule3("dtrans", module_methods,
			  "DistanceTransform module.");
  
  Py_INCREF(&dtrans_type);
  PyModule_AddObject(module, "DistanceTransform", (PyObject *)&dtrans_type);
}
