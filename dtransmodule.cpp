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


static PyObject *
dtrans_isValid(dtrans_object * self, PyObject * args)
{
  unsigned int ix, iy;
  if ( ! PyArg_ParseTuple(args, "II", &ix, &iy)) {
    return NULL;
  }
  if (self->dt->isValid(ix, iy)) {
    Py_RETURN_TRUE;
  }
  Py_RETURN_FALSE;
}


static PyObject *
dtrans_dimX(dtrans_object * self)
{
  return Py_BuildValue("I", self->dt->dimX());
}


static PyObject *
dtrans_dimY(dtrans_object * self)
{
  return Py_BuildValue("I", self->dt->dimY());
}


static PyObject *
dtrans_scale(dtrans_object * self)
{
  return Py_BuildValue("d", self->dt->scale());
}


static PyObject *
dtrans_setDist(dtrans_object * self, PyObject * args)
{
  unsigned int ix, iy;
  double dist;
  if ( ! PyArg_ParseTuple(args, "IId", &ix, &iy, &dist)) {
    return NULL;
  }
  if (self->dt->setDist(ix, iy, dist)) {
    Py_RETURN_TRUE;
  }
  Py_RETURN_FALSE;
}


static PyObject *
dtrans_setSpeed(dtrans_object * self, PyObject * args)
{
  unsigned int ix, iy;
  double speed;
  if ( ! PyArg_ParseTuple(args, "IId", &ix, &iy, &speed)) {
    return NULL;
  }
  if (self->dt->setSpeed(ix, iy, speed)) {
    Py_RETURN_TRUE;
  }
  Py_RETURN_FALSE;
}


static PyObject *
dtrans_getDist(dtrans_object * self, PyObject * args)
{
  unsigned int ix, iy;
  if ( ! PyArg_ParseTuple(args, "II", &ix, &iy)) {
    return NULL;
  }
  return Py_BuildValue("d", self->dt->getDist(ix, iy));
}


// static PyObject *
// dtrans_getSpeed(dtrans_object * self, PyObject * args)
// {
//   unsigned int ix, iy;
//   if ( ! PyArg_ParseTuple(args, "II", &ix, &iy)) {
//     return NULL;
//   }
//   return Py_BuildValue("d", self->dt->getSpeed(ix, iy));
// }


static PyObject *
dtrans_compute(dtrans_object * self, PyObject * args)
{
  double ceiling;
  if ( ! PyArg_ParseTuple(args, "d", &ceiling)) {
    if (PyTuple_Check(args) && (0 != PyTuple_Size(args))) {
      return NULL;
    }
    PyErr_Clear();
    ceiling = dtrans::DistanceTransform::infinity;
  }
  self->dt->compute(ceiling);
  Py_RETURN_NONE;
}


static PyObject *
dtrans_resetDist(dtrans_object * self)
{
  self->dt->resetDist();
  Py_RETURN_NONE;
}


static PyObject *
dtrans_resetSpeed(dtrans_object * self)
{
  self->dt->resetSpeed();
  Py_RETURN_NONE;
}


static PyObject *
dtrans_computeGradient(dtrans_object * self, PyObject * args)
{
  unsigned int ix, iy;
  if ( ! PyArg_ParseTuple(args, "II", &ix, &iy)) {
    return NULL;
  }
  double gx, gy;
  size_t const gn(self->dt->computeGradient(ix, iy, gx, gy));
  return Py_BuildValue("ddI", gx, gy, gn);
}


static PyMethodDef dtrans_methods[] = {
  { "foo", (PyCFunction) dtrans_foo, METH_NOARGS,
    "Return a string combining the dimensions and the scale."
  },
  { "isValid", (PyCFunction) dtrans_isValid, METH_VARARGS,
    "isValid(ix, iy) : returns True if the given grid coordinates lie within the\n"
    "  grid dimensions specified at construction time."
  },
  { "dimX", (PyCFunction) dtrans_dimX, METH_NOARGS,
    "dimX() : returns the number of cells along the X direction."
  },
  { "dimY", (PyCFunction) dtrans_dimY, METH_NOARGS,
    "dimY() : returns the number of cells along the Y direction."
  },
  { "scale", (PyCFunction) dtrans_scale, METH_NOARGS,
    "scale() : returns the length of one side of one cell."
  },
  { "setDist", (PyCFunction) dtrans_setDist, METH_VARARGS,
    "setDist(ix, iy, dist) : set a given cell to a certain distance.\n"
    "\n"
    "  Cells whose distance is set in this manner will be used to seed the distance\n"
    "  transform computation. The propagation will not overwrite a cell's distance\n"
    "  value if it has been set using this method.\n"
    "\n"
    "  Returns True if the cell's distance value has been set (i.e. the given\n"
    "  coordinates lie within the grid)."
  },
  { "setSpeed", (PyCFunction) dtrans_setSpeed, METH_VARARGS,
    "setSpeed(ix, iy, speed) : set the propagation speed for a cell.\n"
    "\n"
    "  Speeds are normalized to the range [0, 1], where zero speed means that the\n"
    "  cell is an obstacle and unit speed means that it is fully in freespace. If you\n"
    "  pass a speed smaller than zero or larger than one, it is ignored and this\n"
    "  method returns false.\n"
    "\n"
    "  NOTE: you should set speeds before propagating the distance transform.\n"
    "        dtrans does not support changing the speed on the fly.\n"
    "\n"
    "  Returns True if the given speed and indices were valid, False otherwise."
  },
  { "getDist", (PyCFunction) dtrans_getDist, METH_VARARGS,
    "getDist(ix, iy) : returns the distance value of a cell. If the cell is invalid\n"
    "  (i.e. it lies outside the grid), then DistanceTransform::infinity is returned."
  },
  { "compute", (PyCFunction) dtrans_compute, METH_VARARGS,
    "compute(ceiling) : propagate the distance transform until a maximum distance has\n"
    "  been reached or the entire grid has been updated. You can call compute() again\n"
    "  with a higher ceiling and it will keep on propagating where it left off. That\n"
    "  way, you	can compute the distance transform in several steps, or	adaptively\n"
    "  change the ceiling depending on whether a certain cell you are interested in\n"
    "  has been reached yet.\n"
    "\n"
    "  NOTE: simply leave off the ceiling parameter if you want to make sure that the\n"
    "        entire grid gets computed. The implementation uses a ceiling of\n"
    "        DistanceTransform::infinity when there is no parameter."
  },
  { "resetDist", (PyCFunction) dtrans_resetDist, METH_NOARGS,
    "resetDist() : reset all distance and gradient data and purge the queue, but keep\n"
    "  the speed map. This is useful if you want to use the DistanceTransform as a\n"
    "  global path planner and reuse a given instance for planning to a new goal."
  },
  { "resetSpeed", (PyCFunction) dtrans_resetSpeed, METH_NOARGS,
    "resetSpeed() : reset the speed map, setting all speeds to 1 (one). If you are\n"
    "  using the DistanceTransform as global planner, this is the same as clearing\n"
    "  all obstacles from the map. This method does not touch the distance or\n"
    "  gradient, nor does it touch the queue. So it really only makes sense to call\n"
    "  it right before or right after calling resetDist()."
  },
  { "computeGradient", (PyCFunction) dtrans_computeGradient, METH_VARARGS,
    "computeGradient(ix, iy) : compute (or look up) the unscaled upwind gradient at a\n"
    "  given cell. Unscaled means that it is not divided by the scale specified at\n"
    "  DistanceTransform construction time, and upwind means that only neighbors lying\n"
    "  below the value of the given cell are taken into account. This makes for faster\n"
    "  and more robust computations.\n"
    "\n"
    "  NOTE: This method caches its results, so calling computeGradient() repeatedly\n"
    "        for a given index does not repeat the computation. If the given (ix, iy)\n"
    "        index is invalid, the returned gradient is zero.\n"
    "\n"
    "  Returns a tuple (gx, gy, gn) where (gx, gy) is the gradient along X and Y, and\n"
    "  gn is the number of neighboring cells taken into account for computing\n"
    "  (gx, gx). If gn is zero, then the gradient is likewise (0, 0) because the cell\n"
    "  is either inside an obstacle, or it is a fixed cell that lies below its\n"
    "  surrounding. Note that obstacle cells with at least one non-obstacle neighbor\n"
    "  will generally result in a non-zero gradient."
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
