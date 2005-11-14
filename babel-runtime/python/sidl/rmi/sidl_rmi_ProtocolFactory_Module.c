/*
 * File:          sidl_rmi_ProtocolFactory_Module.c
 * Symbol:        sidl.rmi.ProtocolFactory-v0.9.3
 * Symbol Type:   class
 * Babel Version: 0.10.12
 * Release:       $Name$
 * Revision:      @(#) $Id$
 * Description:   implement a C extension type for a sidl extendable
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
 * WARNING: Automatically generated; changes will be lost
 * 
 * babel-version = 0.10.12
 */

/*
 * THIS CODE IS AUTOMATICALLY GENERATED BY THE BABEL
 * COMPILER. DO NOT EDIT THIS!
 * 
 * This file contains the implementation of a Python C
 * extension type (i.e. a Python type implemented in C).
 * This extension type provides Python interface to the
 * sidl type sidl.rmi.ProtocolFactory.
 */


/**
 * Symbol "sidl.rmi.ProtocolFactory" (version 0.9.3)
 * 
 * This singleton class keeps a table of string prefixes (e.g. "babel" or "proteus")
 * to protocol implementations.  The intent is to parse a URL (e.g. "babel://server:port/class")
 * and create classes that implement <code>sidl.rmi.InstanceHandle</code>.
 */
#define sidl_rmi_ProtocolFactory_INTERNAL 1
#include "sidl_rmi_ProtocolFactory_Module.h"
#ifndef included_sidl_rmi_ProtocolFactory_IOR_h
#include "sidl_rmi_ProtocolFactory_IOR.h"
#endif
#include "sidlObjA.h"
#include "sidlPyArrays.h"
#include "Numeric/arrayobject.h"
#ifndef included_sidl_Loader_h
#include "sidl_Loader.h"
#endif
#ifndef included_sidl_header_h
#include "sidl_header.h"
#endif
#ifndef included_sidl_interface_IOR_h
#include "sidl_interface_IOR.h"
#endif
#include "sidl_BaseClass_Module.h"
#include "sidl_BaseInterface_Module.h"
#include "sidl_ClassInfo_Module.h"
#include "sidl_rmi_InstanceHandle_Module.h"
#include "sidl_rmi_NetworkException_Module.h"
#include <stdlib.h>
#include <string.h>

staticforward PyTypeObject _sidl_rmi_ProtocolFactoryType;

static struct sidl_rmi_ProtocolFactory__sepv *_sepv = NULL;

static const struct sidl_rmi_ProtocolFactory__external *_implEPV = NULL;

static PyObject *
pStub_ProtocolFactory_createInstance(PyObject *_ignored, PyObject *_args,     \
  PyObject *_kwdict) {
  PyObject *_return_value = NULL;
  char* url = NULL;
  char* typeName = NULL;
  struct sidl_BaseInterface__object *_exception = NULL;
  static char *_kwlist[] = {
    "url",
    "typeName",
    NULL
  };
  const int _okay = PyArg_ParseTupleAndKeywords(
    _args, _kwdict, 
    "zz", _kwlist,
    &url,
    &typeName);
  if (_okay) {
    struct sidl_rmi_InstanceHandle__object* _return = NULL;
    _return = (*(_sepv->f_createInstance))(url, typeName, &_exception);
    if (_exception) {
      struct sidl_rmi_NetworkException__object *_ex0;
      if ((_ex0 = (struct sidl_rmi_NetworkException__object *)
        sidl_PyExceptionCast(_exception, "sidl.rmi.NetworkException")))
      {
        PyObject *obj = sidl_rmi_NetworkException__wrap(_ex0);
        PyObject *_args = PyTuple_New(1);
        PyTuple_SetItem(_args, 0, obj);
        obj = PyObject_CallObject(sidl_rmi_NetworkException__type, _args);
        PyErr_SetObject(sidl_rmi_NetworkException__type, obj);
        Py_XDECREF(_args);
      }
    }
    else {
      _return_value = Py_BuildValue(
        "O&",
        (void *)sidl_rmi_InstanceHandle__wrap, _return);
    }
  }
  return _return_value;
}

static PyObject *
pStub_ProtocolFactory_getProtocol(PyObject *_ignored, PyObject *_args,        \
  PyObject *_kwdict) {
  PyObject *_return_value = NULL;
  char* prefix = NULL;
  struct sidl_BaseInterface__object *_exception = NULL;
  static char *_kwlist[] = {
    "prefix",
    NULL
  };
  const int _okay = PyArg_ParseTupleAndKeywords(
    _args, _kwdict, 
    "z", _kwlist,
    &prefix);
  if (_okay) {
    char* _return = NULL;
    _return = (*(_sepv->f_getProtocol))(prefix, &_exception);
    if (_exception) {
      struct sidl_rmi_NetworkException__object *_ex0;
      if ((_ex0 = (struct sidl_rmi_NetworkException__object *)
        sidl_PyExceptionCast(_exception, "sidl.rmi.NetworkException")))
      {
        PyObject *obj = sidl_rmi_NetworkException__wrap(_ex0);
        PyObject *_args = PyTuple_New(1);
        PyTuple_SetItem(_args, 0, obj);
        obj = PyObject_CallObject(sidl_rmi_NetworkException__type, _args);
        PyErr_SetObject(sidl_rmi_NetworkException__type, obj);
        Py_XDECREF(_args);
      }
    }
    else {
      _return_value = Py_BuildValue(
        "z",
        _return);
    }
    free((void *)_return);
  }
  return _return_value;
}

static PyObject *
pStub_ProtocolFactory_deleteProtocol(PyObject *_ignored, PyObject *_args,     \
  PyObject *_kwdict) {
  PyObject *_return_value = NULL;
  char* prefix = NULL;
  struct sidl_BaseInterface__object *_exception = NULL;
  static char *_kwlist[] = {
    "prefix",
    NULL
  };
  const int _okay = PyArg_ParseTupleAndKeywords(
    _args, _kwdict, 
    "z", _kwlist,
    &prefix);
  if (_okay) {
    sidl_bool _return = (sidl_bool) 0;
    int _proxy__return;
    _return = (*(_sepv->f_deleteProtocol))(prefix, &_exception);
    _proxy__return = _return;
    if (_exception) {
      struct sidl_rmi_NetworkException__object *_ex0;
      if ((_ex0 = (struct sidl_rmi_NetworkException__object *)
        sidl_PyExceptionCast(_exception, "sidl.rmi.NetworkException")))
      {
        PyObject *obj = sidl_rmi_NetworkException__wrap(_ex0);
        PyObject *_args = PyTuple_New(1);
        PyTuple_SetItem(_args, 0, obj);
        obj = PyObject_CallObject(sidl_rmi_NetworkException__type, _args);
        PyErr_SetObject(sidl_rmi_NetworkException__type, obj);
        Py_XDECREF(_args);
      }
    }
    else {
      _return_value = Py_BuildValue(
        "i",
        _proxy__return);
    }
  }
  return _return_value;
}

static PyObject *
pStub_ProtocolFactory_addProtocol(PyObject *_ignored, PyObject *_args,        \
  PyObject *_kwdict) {
  PyObject *_return_value = NULL;
  char* prefix = NULL;
  char* typeName = NULL;
  struct sidl_BaseInterface__object *_exception = NULL;
  static char *_kwlist[] = {
    "prefix",
    "typeName",
    NULL
  };
  const int _okay = PyArg_ParseTupleAndKeywords(
    _args, _kwdict, 
    "zz", _kwlist,
    &prefix,
    &typeName);
  if (_okay) {
    sidl_bool _return = (sidl_bool) 0;
    int _proxy__return;
    _return = (*(_sepv->f_addProtocol))(prefix, typeName, &_exception);
    _proxy__return = _return;
    if (_exception) {
      struct sidl_rmi_NetworkException__object *_ex0;
      if ((_ex0 = (struct sidl_rmi_NetworkException__object *)
        sidl_PyExceptionCast(_exception, "sidl.rmi.NetworkException")))
      {
        PyObject *obj = sidl_rmi_NetworkException__wrap(_ex0);
        PyObject *_args = PyTuple_New(1);
        PyTuple_SetItem(_args, 0, obj);
        obj = PyObject_CallObject(sidl_rmi_NetworkException__type, _args);
        PyErr_SetObject(sidl_rmi_NetworkException__type, obj);
        Py_XDECREF(_args);
      }
    }
    else {
      _return_value = Py_BuildValue(
        "i",
        _proxy__return);
    }
  }
  return _return_value;
}

static PyObject *
pStub_ProtocolFactory_connectInstance(PyObject *_ignored, PyObject *_args,    \
  PyObject *_kwdict) {
  PyObject *_return_value = NULL;
  char* url = NULL;
  struct sidl_BaseInterface__object *_exception = NULL;
  static char *_kwlist[] = {
    "url",
    NULL
  };
  const int _okay = PyArg_ParseTupleAndKeywords(
    _args, _kwdict, 
    "z", _kwlist,
    &url);
  if (_okay) {
    struct sidl_rmi_InstanceHandle__object* _return = NULL;
    _return = (*(_sepv->f_connectInstance))(url, &_exception);
    if (_exception) {
      struct sidl_rmi_NetworkException__object *_ex0;
      if ((_ex0 = (struct sidl_rmi_NetworkException__object *)
        sidl_PyExceptionCast(_exception, "sidl.rmi.NetworkException")))
      {
        PyObject *obj = sidl_rmi_NetworkException__wrap(_ex0);
        PyObject *_args = PyTuple_New(1);
        PyTuple_SetItem(_args, 0, obj);
        obj = PyObject_CallObject(sidl_rmi_NetworkException__type, _args);
        PyErr_SetObject(sidl_rmi_NetworkException__type, obj);
        Py_XDECREF(_args);
      }
    }
    else {
      _return_value = Py_BuildValue(
        "O&",
        (void *)sidl_rmi_InstanceHandle__wrap, _return);
    }
  }
  return _return_value;
}

static int
sidl_rmi_ProtocolFactory_createCast(PyObject *self, PyObject *args,           \
  PyObject *kwds) {
  struct sidl_rmi_ProtocolFactory__object *optarg = NULL;
  static char *_kwlist[] = { "sobj", NULL };
  int _okay = PyArg_ParseTupleAndKeywords(args, kwds, "|O&", _kwlist,         \
    (void *)sidl_rmi_ProtocolFactory__convert, &optarg);
  if (_okay) {
    if (!optarg) {
      optarg = (*(_implEPV->createObject))();
    }
    return sidl_Object_Init(
      (SPObject *)self,
      (struct sidl_BaseInterface__object *)optarg,
      sidl_PyStealRef);
  }
  return -1;
}

static PyMethodDef _ProtocolFactoryModuleMethods[] = {
  { "addProtocol", (PyCFunction)pStub_ProtocolFactory_addProtocol,
  (METH_VARARGS | METH_KEYWORDS),
"\
addProtocol(in string prefix,\n\
            in string typeName)\n\
RETURNS\n\
   (bool _return)\n\
RAISES\n\
    sidl.rmi.NetworkException\n\
\n\
\
Associate a particular prefix in the URL to a typeName <code>sidl.Loader</code> can find.\n\
The actual type is expected to implement <code>sidl.rmi.InstanceHandle</code>\n\
Return true iff the addition is successful.  (no collisions allowed)"
   },
  { "connectInstance", (PyCFunction)pStub_ProtocolFactory_connectInstance,
  (METH_VARARGS | METH_KEYWORDS),
"\
connectInstance(in string url)\n\
RETURNS\n\
   (sidl.rmi.InstanceHandle _return)\n\
RAISES\n\
    sidl.rmi.NetworkException\n\
\n\
\
Create an new connection linked to an already existing object on a remote \n\
server.  The server and port number are in the url, the objectID is the unique ID\n\
of the remote object in the remote instance registry. \n\
Return nil if protocol unknown or InstanceHandle.init() failed."
   },
  { "createInstance", (PyCFunction)pStub_ProtocolFactory_createInstance,
  (METH_VARARGS | METH_KEYWORDS),
"\
createInstance(in string url,\n\
               in string typeName)\n\
RETURNS\n\
   (sidl.rmi.InstanceHandle _return)\n\
RAISES\n\
    sidl.rmi.NetworkException\n\
\n\
\
Create a new remote object and a return an instance handle for that object. \n\
The server and port number are in the url.  Return nil \n\
if protocol unknown or InstanceHandle.init() failed."
   },
  { "deleteProtocol", (PyCFunction)pStub_ProtocolFactory_deleteProtocol,
  (METH_VARARGS | METH_KEYWORDS),
"\
deleteProtocol(in string prefix)\n\
RETURNS\n\
   (bool _return)\n\
RAISES\n\
    sidl.rmi.NetworkException\n\
\n\
\
Remove a protocol from the active list."
   },
  { "getProtocol", (PyCFunction)pStub_ProtocolFactory_getProtocol,
  (METH_VARARGS | METH_KEYWORDS),
"\
getProtocol(in string prefix)\n\
RETURNS\n\
   (string _return)\n\
RAISES\n\
    sidl.rmi.NetworkException\n\
\n\
\
Return the typeName associated with a particular prefix.\n\
Return empty string if the prefix"
   },
  { NULL, NULL }
};

static PyMethodDef _ProtocolFactoryObjectMethods[] = {
  { NULL, NULL }
};

static PyTypeObject _sidl_rmi_ProtocolFactoryType = {
  PyObject_HEAD_INIT(NULL)
  0,      /* ob_size */
  "sidl.rmi.ProtocolFactory.ProtocolFactory", /* tp_name */
  0,      /* tp_basicsize */
  0,      /* tp_itemsize */
  0,      /* tp_dealloc */
  0,      /* tp_print */
  0,      /* tp_getattr */
  0,      /* tp_setattr */
  0,      /* tp_compare */
  0,      /* tp_repr */
  0,      /* tp_as_number */
  0,      /* tp_as_sequence */
  0,      /* tp_as_mapping */
  0,      /* tp_hash  */
  0,      /* tp_call */
  0,      /* tp_str */
  0,      /* tp_getattro */
  0,      /* tp_setattro */
  0,      /* tp_as_buffer */
  Py_TPFLAGS_DEFAULT, /* tp_flags */
  "\
\
This singleton class keeps a table of string prefixes (e.g. \"babel\" or \"proteus\")\n\
to protocol implementations.  The intent is to parse a URL (e.g. \"babel://server:port/class\")\n\
and create classes that implement <code>sidl.rmi.InstanceHandle</code>.", /* tp_doc */
  0,      /* tp_traverse */
  0,       /* tp_clear */
  0,       /* tp_richcompare */
  0,       /* tp_weaklistoffset */
  0,       /* tp_iter */
  0,       /* tp_iternext */
  _ProtocolFactoryObjectMethods, /* tp_methods */
  0,       /* tp_members */
  0,       /* tp_getset */
  0,       /* tp_base */
  0,       /* tp_dict */
  0,       /* tp_descr_get */
  0,       /* tp_descr_set */
  0,       /* tp_dictoffset */
  sidl_rmi_ProtocolFactory_createCast,   /* tp_init */
  0,       /* tp_alloc */
  0,       /* tp_new */
};

sidl_rmi_ProtocolFactory__wrap_RETURN
sidl_rmi_ProtocolFactory__wrap sidl_rmi_ProtocolFactory__wrap_PROTO {
  PyObject *result;
  if (sidlobj) {
    result =                                                                  \
      _sidl_rmi_ProtocolFactoryType.tp_new(&_sidl_rmi_ProtocolFactoryType,    \
      NULL, NULL);
    if (result) {
      if (sidl_Object_Init(
        (SPObject *)result,
        (struct sidl_BaseInterface__object *)(sidlobj),
        sidl_PyStealRef))
      {
        Py_DECREF(result);
        result = NULL;
      }
    }
  }
  else {
    result = Py_None;
    Py_INCREF(result);
  }
  return result;
}

sidl_rmi_ProtocolFactory__weakRef_RETURN
sidl_rmi_ProtocolFactory__weakRef sidl_rmi_ProtocolFactory__weakRef_PROTO {
  PyObject *result;
  if (sidlobj) {
    result =                                                                  \
      _sidl_rmi_ProtocolFactoryType.tp_new(&_sidl_rmi_ProtocolFactoryType,    \
      NULL, NULL);
    if (result) {
      if (sidl_Object_Init(
        (SPObject *)result,
        (struct sidl_BaseInterface__object *)(sidlobj),
        sidl_PyWeakRef))
      {
        Py_DECREF(result);
        result = NULL;
      }
    }
  }
  else {
    result = Py_None;
    Py_INCREF(result);
  }
  return result;
}

sidl_rmi_ProtocolFactory_deref_RETURN
sidl_rmi_ProtocolFactory_deref sidl_rmi_ProtocolFactory_deref_PROTO {
  if (sidlobj) {
    (*(sidlobj->d_epv->f_deleteRef))(sidlobj);
  }
}

sidl_rmi_ProtocolFactory__newRef_RETURN
sidl_rmi_ProtocolFactory__newRef sidl_rmi_ProtocolFactory__newRef_PROTO {
  PyObject *result;
  if (sidlobj) {
    result =                                                                  \
      _sidl_rmi_ProtocolFactoryType.tp_new(&_sidl_rmi_ProtocolFactoryType,    \
      NULL, NULL);
    if (result) {
      if (sidl_Object_Init(
        (SPObject *)result,
        (struct sidl_BaseInterface__object *)(sidlobj),
        sidl_PyNewRef))
      {
        Py_DECREF(result);
        result = NULL;
      }
    }
  }
  else {
    result = Py_None;
    Py_INCREF(result);
  }
  return result;
}

sidl_rmi_ProtocolFactory__addRef_RETURN
sidl_rmi_ProtocolFactory__addRef sidl_rmi_ProtocolFactory__addRef_PROTO {
  if (sidlobj) {
    (*(sidlobj->d_epv->f_addRef))(sidlobj);
  }
}

sidl_rmi_ProtocolFactory_PyType_RETURN
sidl_rmi_ProtocolFactory_PyType sidl_rmi_ProtocolFactory_PyType_PROTO {
  Py_INCREF(&_sidl_rmi_ProtocolFactoryType);
  return &_sidl_rmi_ProtocolFactoryType;
}

sidl_rmi_ProtocolFactory__convert_RETURN
sidl_rmi_ProtocolFactory__convert sidl_rmi_ProtocolFactory__convert_PROTO {
  *sidlobj = sidl_Cast(obj, "sidl.rmi.ProtocolFactory");
  if (*sidlobj) {
    (*((*sidlobj)->d_epv->f_addRef))(*sidlobj);
  }
  else if (obj != Py_None) {
    PyErr_SetString(PyExc_TypeError, 
      "argument is not a(n) sidl.rmi.ProtocolFactory");
    return 0;
  }
  return 1;
}

static int
_convertPython(void *sidlarray, const int *ind, PyObject *pyobj)
{
  struct sidl_rmi_ProtocolFactory__object *sidlobj;
  if (sidl_rmi_ProtocolFactory__convert(pyobj, &sidlobj)) {
    sidl_interface__array_set((struct sidl_interface__array *)sidlarray,
    ind, (struct sidl_BaseInterface__object *)sidlobj);
    if (sidlobj) {
      sidl_BaseInterface_deleteRef((struct sidl_BaseInterface__object         \
        *)sidlobj);
    }
    return FALSE;
  }
  return TRUE;
}

sidl_rmi_ProtocolFactory__convert_python_array_RETURN
sidl_rmi_ProtocolFactory__convert_python_array                                \
  sidl_rmi_ProtocolFactory__convert_python_array_PROTO {
  int result = 0;
  *sidlarray = NULL;
  if (obj == Py_None) {
    result = TRUE;
  }
  else {
    PyObject *pya = PyArray_FromObject(obj, PyArray_OBJECT, 0, 0);
    if (pya) {
      if (PyArray_OBJECT == ((PyArrayObject *)pya)->descr->type_num) {
        int dimen, lower[SIDL_MAX_ARRAY_DIMENSION],
          upper[SIDL_MAX_ARRAY_DIMENSION],
          stride[SIDL_MAX_ARRAY_DIMENSION];
        if (sidl_array__extract_python_info
          (pya, &dimen, lower, upper, stride))
        {
            *sidlarray = (struct                                              \
              sidl_rmi_ProtocolFactory__array*)sidl_interface__array_createRow
            (dimen, lower, upper);
          result = sidl_array__convert_python
            (pya, dimen, *sidlarray, _convertPython);
          if (*sidlarray && !result) {
            sidl_interface__array_deleteRef(
              (struct  sidl_interface__array *)*sidlarray);
            *sidlarray = NULL;
          }
        }
      }
      Py_DECREF(pya);
    }
  }
  return result;
}

static int
_convertSIDL(void *sidlarray, const int *ind, PyObject **dest)
{
  struct sidl_rmi_ProtocolFactory__object *sidlobj = (struct                  \
    sidl_rmi_ProtocolFactory__object*)
  sidl_interface__array_get((struct sidl_interface__array *)
    sidlarray, ind);
  *dest = sidl_rmi_ProtocolFactory__wrap(sidlobj);
  return (*dest == NULL);
}

sidl_rmi_ProtocolFactory__convert_sidl_array_RETURN
sidl_rmi_ProtocolFactory__convert_sidl_array                                  \
  sidl_rmi_ProtocolFactory__convert_sidl_array_PROTO {
  PyObject *pya = NULL;
  if (sidlarray) {
    const int dimen = sidlArrayDim(sidlarray);
    int i;
    int *lower = (int *)malloc(sizeof(int) * dimen);
    int *upper = (int *)malloc(sizeof(int) * dimen);
    int *numelem = (int *)malloc(sizeof(int) * dimen);
    for(i = 0; i < dimen; ++i) {
      lower[i] = sidlLower(sidlarray, i);
      upper[i] = sidlUpper(sidlarray, i);
      numelem[i] = 1 + upper[i] - lower[i];
    }
    pya = PyArray_FromDims(dimen, numelem, PyArray_OBJECT);
    if (pya) {
      if (!sidl_array__convert_sidl(pya, dimen, lower, upper,
        numelem, sidlarray, _convertSIDL))
      {
        Py_DECREF(pya);
        pya = NULL;
      }
    }
    free(numelem);
    free(upper);
    free(lower);
  }
  else {
    Py_INCREF(Py_None);
    pya = Py_None;
  }
  return pya;
}

void
initProtocolFactory(void) {
  PyObject *module, *dict, *c_api;
  static void *ExternalAPI[sidl_rmi_ProtocolFactory__API_NUM];
  module = Py_InitModule3("ProtocolFactory", _ProtocolFactoryModuleMethods, "\
\
This singleton class keeps a table of string prefixes (e.g. \"babel\" or \"proteus\")\n\
to protocol implementations.  The intent is to parse a URL (e.g. \"babel://server:port/class\")\n\
and create classes that implement <code>sidl.rmi.InstanceHandle</code>."
  );
  dict = PyModule_GetDict(module);
  ExternalAPI[sidl_rmi_ProtocolFactory__wrap_NUM] =                           \
    (void*)sidl_rmi_ProtocolFactory__wrap;
  ExternalAPI[sidl_rmi_ProtocolFactory__convert_NUM] =                        \
    (void*)sidl_rmi_ProtocolFactory__convert;
  ExternalAPI[sidl_rmi_ProtocolFactory__convert_python_array_NUM] =           \
    (void*)sidl_rmi_ProtocolFactory__convert_python_array;
  ExternalAPI[sidl_rmi_ProtocolFactory__convert_sidl_array_NUM] =             \
    (void*)sidl_rmi_ProtocolFactory__convert_sidl_array;
  ExternalAPI[sidl_rmi_ProtocolFactory__weakRef_NUM] =                        \
    (void*)sidl_rmi_ProtocolFactory__weakRef;
  ExternalAPI[sidl_rmi_ProtocolFactory_deref_NUM] =                           \
    (void*)sidl_rmi_ProtocolFactory_deref;
  ExternalAPI[sidl_rmi_ProtocolFactory__newRef_NUM] =                         \
    (void*)sidl_rmi_ProtocolFactory__newRef;
  ExternalAPI[sidl_rmi_ProtocolFactory__addRef_NUM] =                         \
    (void*)sidl_rmi_ProtocolFactory__addRef;
  ExternalAPI[sidl_rmi_ProtocolFactory_PyType_NUM] =                          \
    (void*)sidl_rmi_ProtocolFactory_PyType;
  import_SIDLObjA();
  if (PyErr_Occurred()) {
    Py_FatalError("Error importing sidlObjA module.");
  }
  c_api = PyCObject_FromVoidPtr((void *)ExternalAPI, NULL);
  PyDict_SetItemString(dict, "_C_API", c_api);
  Py_XDECREF(c_api);
  import_SIDLPyArrays();
  if (PyErr_Occurred()) {
    Py_FatalError("Error importing sidlPyArrays module.");
  }
  import_array();
  if (PyErr_Occurred()) {
    Py_FatalError("Error importing Numeric Python module.");
  }
  sidl_BaseClass__import();
  _sidl_rmi_ProtocolFactoryType.tp_base = sidl_BaseClass_PyType();
  _sidl_rmi_ProtocolFactoryType.tp_bases = PyTuple_New(1);
  PyTuple_SetItem(_sidl_rmi_ProtocolFactoryType.tp_bases,0,                   \
    (PyObject *)sidl_BaseClass_PyType());
  if (PyType_Ready(&_sidl_rmi_ProtocolFactoryType) < 0) {
    PyErr_Print();
    fprintf(stderr, "PyType_Ready on sidl.rmi.ProtocolFactory failed.\n");
    return;
  }
  Py_INCREF(&_sidl_rmi_ProtocolFactoryType);
  PyDict_SetItemString(dict, "ProtocolFactory",                               \
    (PyObject *)&_sidl_rmi_ProtocolFactoryType);
  sidl_ClassInfo__import();
  sidl_BaseInterface__import();
  sidl_rmi_InstanceHandle__import();
  sidl_rmi_NetworkException__import();
  _implEPV = sidl_rmi_ProtocolFactory__externals();
  if (_implEPV) {
    _sepv = (*_implEPV->getStaticEPV)();
    if (PyErr_Occurred()) {
      Py_FatalError("Cannot initialize Python module                          \
        sidl.rmi.ProtocolFactory.");
    }
  }
  else {
    Py_FatalError("Cannot load implementation for sidl class                  \
      sidl.rmi.ProtocolFactory");
  }
}
