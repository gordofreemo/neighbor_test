/*
 * File:          sidl_ClassInfo_Module.h
 * Symbol:        sidl.ClassInfo-v0.9.3
 * Symbol Type:   interface
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
 * to an instance of sidl.ClassInfo.
 * This header files defines two methods that such clients
 * will need.
 *     sidl_ClassInfo__import
 *         This should be called in the client's init
 *         module method.
 *     sidl_ClassInfo__wrap
 *         This will wrap an IOR in a Python object.
 */

#ifndef included_sidl_ClassInfo_MODULE
#define included_sidl_ClassInfo_MODULE

#include <Python.h>
#include "babel_config.h"

#ifdef __cplusplus
extern "C" {
#endif

struct sidl__array;

/* Forward declaration of IOR structure */
struct sidl_ClassInfo__object;
struct sidl_ClassInfo__array;

#define sidl_ClassInfo__wrap_NUM 0
#define sidl_ClassInfo__wrap_RETURN PyObject *
#define sidl_ClassInfo__wrap_PROTO (struct sidl_ClassInfo__object *sidlobj)

#define sidl_ClassInfo__convert_NUM 1
#define sidl_ClassInfo__convert_RETURN int
#define sidl_ClassInfo__convert_PROTO (PyObject *obj, struct sidl_ClassInfo__object **sidlobj)

#define sidl_ClassInfo__convert_python_array_NUM 2
#define sidl_ClassInfo__convert_python_array_RETURN int
#define sidl_ClassInfo__convert_python_array_PROTO (PyObject *obj, struct sidl_ClassInfo__array **sidlarray)

#define sidl_ClassInfo__convert_sidl_array_NUM 3
#define sidl_ClassInfo__convert_sidl_array_RETURN PyObject *
#define sidl_ClassInfo__convert_sidl_array_PROTO (struct sidl__array *sidlarray)

#define sidl_ClassInfo__weakRef_NUM 4
#define sidl_ClassInfo__weakRef_RETURN PyObject *
#define sidl_ClassInfo__weakRef_PROTO (struct sidl_ClassInfo__object *sidlobj)

#define sidl_ClassInfo_deref_NUM 5
#define sidl_ClassInfo_deref_RETURN void
#define sidl_ClassInfo_deref_PROTO (struct sidl_ClassInfo__object *sidlobj)

#define sidl_ClassInfo__newRef_NUM 6
#define sidl_ClassInfo__newRef_RETURN PyObject *
#define sidl_ClassInfo__newRef_PROTO (struct sidl_ClassInfo__object *sidlobj)

#define sidl_ClassInfo__addRef_NUM 7
#define sidl_ClassInfo__addRef_RETURN void
#define sidl_ClassInfo__addRef_PROTO (struct sidl_ClassInfo__object *sidlobj)

#define sidl_ClassInfo_PyType_NUM 8
#define sidl_ClassInfo_PyType_RETURN PyTypeObject *
#define sidl_ClassInfo_PyType_PROTO (void)

#define sidl_ClassInfo__API_NUM 9

#ifdef sidl_ClassInfo_INTERNAL

/*
 * This declaration is not for clients.
 */

static sidl_ClassInfo__wrap_RETURN
sidl_ClassInfo__wrap
sidl_ClassInfo__wrap_PROTO;

static sidl_ClassInfo__convert_RETURN
sidl_ClassInfo__convert
sidl_ClassInfo__convert_PROTO;

static sidl_ClassInfo__convert_python_array_RETURN
sidl_ClassInfo__convert_python_array
sidl_ClassInfo__convert_python_array_PROTO;

static sidl_ClassInfo__convert_sidl_array_RETURN
sidl_ClassInfo__convert_sidl_array
sidl_ClassInfo__convert_sidl_array_PROTO;

static sidl_ClassInfo__weakRef_RETURN
sidl_ClassInfo__weakRef
sidl_ClassInfo__weakRef_PROTO;

static sidl_ClassInfo_deref_RETURN
sidl_ClassInfo_deref
sidl_ClassInfo_deref_PROTO;

static sidl_ClassInfo__newRef_RETURN
sidl_ClassInfo__newRef
sidl_ClassInfo__newRef_PROTO;

static sidl_ClassInfo__addRef_RETURN
sidl_ClassInfo__addRef
sidl_ClassInfo__addRef_PROTO;

static sidl_ClassInfo_PyType_RETURN
sidl_ClassInfo_PyType
sidl_ClassInfo_PyType_PROTO;

#else

static void **sidl_ClassInfo__API;

#define sidl_ClassInfo__wrap \
  (*((sidl_ClassInfo__wrap_RETURN (*) \
  sidl_ClassInfo__wrap_PROTO) \
  (sidl_ClassInfo__API \
  [sidl_ClassInfo__wrap_NUM])))

#define sidl_ClassInfo__convert \
  (*((sidl_ClassInfo__convert_RETURN (*) \
  sidl_ClassInfo__convert_PROTO) \
  (sidl_ClassInfo__API \
  [sidl_ClassInfo__convert_NUM])))

#define sidl_ClassInfo__convert_python_array \
  (*((sidl_ClassInfo__convert_python_array_RETURN (*) \
  sidl_ClassInfo__convert_python_array_PROTO) \
  (sidl_ClassInfo__API \
  [sidl_ClassInfo__convert_python_array_NUM])))

#define sidl_ClassInfo__convert_sidl_array \
  (*((sidl_ClassInfo__convert_sidl_array_RETURN (*) \
  sidl_ClassInfo__convert_sidl_array_PROTO) \
  (sidl_ClassInfo__API \
  [sidl_ClassInfo__convert_sidl_array_NUM])))

#define sidl_ClassInfo__weakRef \
  (*((sidl_ClassInfo__weakRef_RETURN (*) \
  sidl_ClassInfo__weakRef_PROTO) \
  (sidl_ClassInfo__API \
  [sidl_ClassInfo__weakRef_NUM])))

#define sidl_ClassInfo_deref \
  (*((sidl_ClassInfo_deref_RETURN (*) \
  sidl_ClassInfo_deref_PROTO) \
  (sidl_ClassInfo__API \
  [sidl_ClassInfo_deref_NUM])))

#define sidl_ClassInfo__newRef \
  (*((sidl_ClassInfo__newRef_RETURN (*) \
  sidl_ClassInfo__newRef_PROTO) \
  (sidl_ClassInfo__API \
  [sidl_ClassInfo__newRef_NUM])))

#define sidl_ClassInfo__addRef \
  (*((sidl_ClassInfo__addRef_RETURN (*) \
  sidl_ClassInfo__addRef_PROTO) \
  (sidl_ClassInfo__API \
  [sidl_ClassInfo__addRef_NUM])))

#define sidl_ClassInfo_PyType \
  (*((sidl_ClassInfo_PyType_RETURN (*) \
  sidl_ClassInfo_PyType_PROTO) \
  (sidl_ClassInfo__API \
  [sidl_ClassInfo_PyType_NUM])))

#define sidl_ClassInfo__import() \
{ \
  PyObject *module = PyImport_ImportModule("sidl.ClassInfo"); \
  if (module != NULL) { \
    PyObject *module_dict = PyModule_GetDict(module); \
    PyObject *c_api_object = \
      PyDict_GetItemString(module_dict, "_C_API"); \
    if (c_api_object && PyCObject_Check(c_api_object)) { \
      sidl_ClassInfo__API = \
        (void **)PyCObject_AsVoidPtr(c_api_object); \
    } \
    else { fprintf(stderr, "babel: sidl_ClassInfo__import failed to lookup _C_API (%p %p %s).\n", c_api_object, c_api_object ? c_api_object->ob_type : NULL, c_api_object ? c_api_object->ob_type->tp_name : ""); }\
    Py_DECREF(module); \
  } else { fprintf(stderr, "babel: sidl_ClassInfo__import failed to import its module.\n"); }\
}

#endif

#ifdef __cplusplus
}
#endif

#endif
