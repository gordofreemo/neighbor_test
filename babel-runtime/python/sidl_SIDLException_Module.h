/*
 * File:          sidl_SIDLException_Module.h
 * Symbol:        sidl.SIDLException-v0.9.3
 * Symbol Type:   class
 * Babel Version: 0.10.12
 * Release:       $Name$
 * Revision:      @(#) $Id$
 * Description:   expose a constructor for the Python wrapper
 * 
 * Copyright (c) 2000-2002, The Regents of the University of California.
 * Produced at the Lawrence Livermore National Laboratory.
 * Written by the Components Team <components@llnl.gov>
 * All rights reserved.
 * 
 * This file is part of Babel. For more information, see
 * http://www.llnl.gov/CASC/components/. Please read the COPYRIGHT file
 * for Our Notice and the LICENSE file for the GNU Lesser General Public
 * License.
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License (as published by
 * the Free Software Foundation) version 2.1 dated February 1999.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the IMPLIED WARRANTY OF
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the terms and
 * conditions of the GNU Lesser General Public License for more details.
 * 
 * You should have recieved a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * 
 * WARNING: Automatically generated; only changes within splicers preserved
 * 
 * babel-version = 0.10.12
 */

/*
 * THIS CODE IS AUTOMATICALLY GENERATED BY THE BABEL
 * COMPILER. DO NOT EDIT THIS!
 * 
 * External clients need an entry point to wrap a pointer
 * to an instance of sidl.SIDLException.
 * This header files defines two methods that such clients
 * will need.
 *     sidl_SIDLException__import
 *         This should be called in the client's init
 *         module method.
 *     sidl_SIDLException__wrap
 *         This will wrap an IOR in a Python object.
 * This object can be used as an exception. It exports
 * a Python exception type that may be needed as well.
 *     sidl_SIDLException__type
 *         A Python exception type corresponding to
 *         this object type.
 * Here is the pattern for throwing an exception:
 *   PyObject *obj = sidl_SIDLException__wrap(ex);
 *   PyErr_SetObject(sidl_SIDLException__type, obj);
 *   Py_XDECREF(obj);
 */

#ifndef included_sidl_SIDLException_MODULE
#define included_sidl_SIDLException_MODULE

#include <Python.h>
#include "babel_config.h"

#ifdef __cplusplus
extern "C" {
#endif

struct sidl__array;

/* Forward declaration of IOR structure */
struct sidl_SIDLException__object;
struct sidl_SIDLException__array;

#define sidl_SIDLException__wrap_NUM 0
#define sidl_SIDLException__wrap_RETURN PyObject *
#define sidl_SIDLException__wrap_PROTO (struct sidl_SIDLException__object *sidlobj)

#define sidl_SIDLException__convert_NUM 1
#define sidl_SIDLException__convert_RETURN int
#define sidl_SIDLException__convert_PROTO (PyObject *obj, struct sidl_SIDLException__object **sidlobj)

#define sidl_SIDLException__convert_python_array_NUM 2
#define sidl_SIDLException__convert_python_array_RETURN int
#define sidl_SIDLException__convert_python_array_PROTO (PyObject *obj, struct sidl_SIDLException__array **sidlarray)

#define sidl_SIDLException__convert_sidl_array_NUM 3
#define sidl_SIDLException__convert_sidl_array_RETURN PyObject *
#define sidl_SIDLException__convert_sidl_array_PROTO (struct sidl__array *sidlarray)

#define sidl_SIDLException__weakRef_NUM 4
#define sidl_SIDLException__weakRef_RETURN PyObject *
#define sidl_SIDLException__weakRef_PROTO (struct sidl_SIDLException__object *sidlobj)

#define sidl_SIDLException_deref_NUM 5
#define sidl_SIDLException_deref_RETURN void
#define sidl_SIDLException_deref_PROTO (struct sidl_SIDLException__object *sidlobj)

#define sidl_SIDLException__newRef_NUM 6
#define sidl_SIDLException__newRef_RETURN PyObject *
#define sidl_SIDLException__newRef_PROTO (struct sidl_SIDLException__object *sidlobj)

#define sidl_SIDLException__addRef_NUM 7
#define sidl_SIDLException__addRef_RETURN void
#define sidl_SIDLException__addRef_PROTO (struct sidl_SIDLException__object *sidlobj)

#define sidl_SIDLException_PyType_NUM 8
#define sidl_SIDLException_PyType_RETURN PyTypeObject *
#define sidl_SIDLException_PyType_PROTO (void)

#define sidl_SIDLException__type_NUM 9

#define sidl_SIDLException__API_NUM 10

#ifdef sidl_SIDLException_INTERNAL

/*
 * This declaration is not for clients.
 */

static sidl_SIDLException__wrap_RETURN
sidl_SIDLException__wrap
sidl_SIDLException__wrap_PROTO;

static sidl_SIDLException__convert_RETURN
sidl_SIDLException__convert
sidl_SIDLException__convert_PROTO;

static sidl_SIDLException__convert_python_array_RETURN
sidl_SIDLException__convert_python_array
sidl_SIDLException__convert_python_array_PROTO;

static sidl_SIDLException__convert_sidl_array_RETURN
sidl_SIDLException__convert_sidl_array
sidl_SIDLException__convert_sidl_array_PROTO;

static sidl_SIDLException__weakRef_RETURN
sidl_SIDLException__weakRef
sidl_SIDLException__weakRef_PROTO;

static sidl_SIDLException_deref_RETURN
sidl_SIDLException_deref
sidl_SIDLException_deref_PROTO;

static sidl_SIDLException__newRef_RETURN
sidl_SIDLException__newRef
sidl_SIDLException__newRef_PROTO;

static sidl_SIDLException__addRef_RETURN
sidl_SIDLException__addRef
sidl_SIDLException__addRef_PROTO;

static sidl_SIDLException_PyType_RETURN
sidl_SIDLException_PyType
sidl_SIDLException_PyType_PROTO;

static PyObject *
sidl_SIDLException__type;

#else

static void **sidl_SIDLException__API;

#define sidl_SIDLException__wrap \
  (*((sidl_SIDLException__wrap_RETURN (*) \
  sidl_SIDLException__wrap_PROTO) \
  (sidl_SIDLException__API \
  [sidl_SIDLException__wrap_NUM])))

#define sidl_SIDLException__convert \
  (*((sidl_SIDLException__convert_RETURN (*) \
  sidl_SIDLException__convert_PROTO) \
  (sidl_SIDLException__API \
  [sidl_SIDLException__convert_NUM])))

#define sidl_SIDLException__convert_python_array \
  (*((sidl_SIDLException__convert_python_array_RETURN (*) \
  sidl_SIDLException__convert_python_array_PROTO) \
  (sidl_SIDLException__API \
  [sidl_SIDLException__convert_python_array_NUM])))

#define sidl_SIDLException__convert_sidl_array \
  (*((sidl_SIDLException__convert_sidl_array_RETURN (*) \
  sidl_SIDLException__convert_sidl_array_PROTO) \
  (sidl_SIDLException__API \
  [sidl_SIDLException__convert_sidl_array_NUM])))

#define sidl_SIDLException__weakRef \
  (*((sidl_SIDLException__weakRef_RETURN (*) \
  sidl_SIDLException__weakRef_PROTO) \
  (sidl_SIDLException__API \
  [sidl_SIDLException__weakRef_NUM])))

#define sidl_SIDLException_deref \
  (*((sidl_SIDLException_deref_RETURN (*) \
  sidl_SIDLException_deref_PROTO) \
  (sidl_SIDLException__API \
  [sidl_SIDLException_deref_NUM])))

#define sidl_SIDLException__newRef \
  (*((sidl_SIDLException__newRef_RETURN (*) \
  sidl_SIDLException__newRef_PROTO) \
  (sidl_SIDLException__API \
  [sidl_SIDLException__newRef_NUM])))

#define sidl_SIDLException__addRef \
  (*((sidl_SIDLException__addRef_RETURN (*) \
  sidl_SIDLException__addRef_PROTO) \
  (sidl_SIDLException__API \
  [sidl_SIDLException__addRef_NUM])))

#define sidl_SIDLException_PyType \
  (*((sidl_SIDLException_PyType_RETURN (*) \
  sidl_SIDLException_PyType_PROTO) \
  (sidl_SIDLException__API \
  [sidl_SIDLException_PyType_NUM])))

#define sidl_SIDLException__type \
  ((PyObject *)(sidl_SIDLException__API[sidl_SIDLException__type_NUM]))

#define sidl_SIDLException__import() \
{ \
  PyObject *module = PyImport_ImportModule("sidl.SIDLException"); \
  if (module != NULL) { \
    PyObject *module_dict = PyModule_GetDict(module); \
    PyObject *c_api_object = \
      PyDict_GetItemString(module_dict, "_C_API"); \
    if (c_api_object && PyCObject_Check(c_api_object)) { \
      sidl_SIDLException__API = \
        (void **)PyCObject_AsVoidPtr(c_api_object); \
    } \
    else { fprintf(stderr, "babel: sidl_SIDLException__import failed to lookup _C_API (%p %p %s).\n", c_api_object, c_api_object ? c_api_object->ob_type : NULL, c_api_object ? c_api_object->ob_type->tp_name : ""); }\
    Py_DECREF(module); \
  } else { fprintf(stderr, "babel: sidl_SIDLException__import failed to import its module.\n"); }\
}

#endif

#ifdef __cplusplus
}
#endif

#endif
