/***********************************************************************
 * python.swg
 *
 *     This file contains the runtime support for Python modules
 *     and includes code for managing global variables and pointer
 *     type checking.
 *
 * Author : David Beazley (beazley@cs.uchicago.edu)
 ************************************************************************/

#include <stdlib.h>
#include "Python.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SWIG_PY_INT     1
#define SWIG_PY_FLOAT   2
#define SWIG_PY_STRING  3
#define SWIG_PY_POINTER 4
#define SWIG_PY_BINARY  5

/* Constant information structure */
typedef struct swig_const_info {
    int type;
    char *name;
    long lvalue;
    double dvalue;
    void   *pvalue;
    swig_type_info **ptype;
} swig_const_info;

#ifdef SWIG_NOINCLUDE

SWIGEXPORT(PyObject *)        SWIG_newvarlink();
SWIGEXPORT(void)              SWIG_addvarlink(PyObject *, char *, PyObject *(*)(void), int (*)(PyObject *));
SWIGEXPORT(int)               SWIG_ConvertPtr(PyObject *, void **, swig_type_info *, int);
SWIGEXPORT(int)               SWIG_ConvertPacked(PyObject *, void *, int sz, swig_type_info *, int);
SWIGEXPORT(char *)            SWIG_PackData(char *c, void *, int);
SWIGEXPORT(char *)            SWIG_UnpackData(char *c, void *, int);
SWIGEXPORT(PyObject *)        SWIG_NewPointerObj(void *, swig_type_info *,int own);
SWIGEXPORT(PyObject *)        SWIG_NewPackedObj(void *, int sz, swig_type_info *);
SWIGEXPORT(void)              SWIG_InstallConstants(PyObject *d, swig_const_info constants[]);
SWIGEXPORT(PyObject *)        SWIG_MakeShadow(PyObject *robj, swig_type_info *type, int own);
#else

/* -----------------------------------------------------------------------------
 * global variable support code.
 * ----------------------------------------------------------------------------- */

typedef struct swig_globalvar {   
  char       *name;                  /* Name of global variable */
  PyObject *(*get_attr)(void);       /* Return the current value */
  int       (*set_attr)(PyObject *); /* Set the value */
  struct swig_globalvar *next;
} swig_globalvar;

typedef struct swig_varlinkobject {
  PyObject_HEAD
  swig_globalvar *vars;
} swig_varlinkobject;

static PyObject *
swig_varlink_repr(swig_varlinkobject *v) {
  v = v;
  return PyString_FromString("<Global variables>");
}

static int
swig_varlink_print(swig_varlinkobject *v, FILE *fp, int flags) {
  swig_globalvar  *var;
  flags = flags;
  fprintf(fp,"Global variables { ");
  for (var = v->vars; var; var=var->next) {
    fprintf(fp,"%s", var->name);
    if (var->next) fprintf(fp,", ");
  }
  fprintf(fp," }\n");
  return 0;
}

static PyObject *
swig_varlink_getattr(swig_varlinkobject *v, char *n) {
  swig_globalvar *var = v->vars;
  while (var) {
    if (strcmp(var->name,n) == 0) {
      return (*var->get_attr)();
    }
    var = var->next;
  }
  PyErr_SetString(PyExc_NameError,"Unknown C global variable");
  return NULL;
}

static int
swig_varlink_setattr(swig_varlinkobject *v, char *n, PyObject *p) {
  swig_globalvar *var = v->vars;
  while (var) {
    if (strcmp(var->name,n) == 0) {
      return (*var->set_attr)(p);
    }
    var = var->next;
  }
  PyErr_SetString(PyExc_NameError,"Unknown C global variable");
  return 1;
}

statichere PyTypeObject varlinktype = {
  PyObject_HEAD_INIT(0)              
  0,
  (char *)"swigvarlink",                      /* Type name    */
  sizeof(swig_varlinkobject),         /* Basic size   */
  0,                                  /* Itemsize     */
  0,                                  /* Deallocator  */ 
  (printfunc) swig_varlink_print,     /* Print        */
  (getattrfunc) swig_varlink_getattr, /* get attr     */
  (setattrfunc) swig_varlink_setattr, /* Set attr     */
  0,                                  /* tp_compare   */
  (reprfunc) swig_varlink_repr,       /* tp_repr      */    
  0,                                  /* tp_as_number */
  0,                                  /* tp_as_mapping*/
  0,                                  /* tp_hash      */
};

/* Create a variable linking object for use later */
SWIGRUNTIME(PyObject *)
SWIG_newvarlink(void) {
  swig_varlinkobject *result = 0;
  result = PyMem_NEW(swig_varlinkobject,1);
  varlinktype.ob_type = &PyType_Type;    /* Patch varlinktype into a PyType */
  result->ob_type = &varlinktype;
  result->vars = 0;
  result->ob_refcnt = 0;
  Py_XINCREF((PyObject *) result);
  return ((PyObject*) result);
}

SWIGRUNTIME(void)
SWIG_addvarlink(PyObject *p, char *name,
	   PyObject *(*get_attr)(void), int (*set_attr)(PyObject *p)) {
  swig_varlinkobject *v;
  swig_globalvar *gv;
  v= (swig_varlinkobject *) p;
  gv = (swig_globalvar *) malloc(sizeof(swig_globalvar));
  gv->name = (char *) malloc(strlen(name)+1);
  strcpy(gv->name,name);
  gv->get_attr = get_attr;
  gv->set_attr = set_attr;
  gv->next = v->vars;
  v->vars = gv;
}

/* Pack binary data into a string */
SWIGRUNTIME(char *)
SWIG_PackData(char *c, void *ptr, int sz) {
  static char hex[17] = "0123456789abcdef";
  int i;
  unsigned char *u = (unsigned char *) ptr;
  register unsigned char uu;
  for (i = 0; i < sz; i++,u++) {
    uu = *u;
    *(c++) = hex[(uu & 0xf0) >> 4];
    *(c++) = hex[uu & 0xf];
  }
  return c;
}

/* Unpack binary data from a string */
SWIGRUNTIME(char *)
SWIG_UnpackData(char *c, void *ptr, int sz) {
  register unsigned char uu;
  register int d;
  unsigned char *u = (unsigned char *) ptr;
  int i;
  for (i = 0; i < sz; i++, u++) {
    d = *(c++);
    if ((d >= '0') && (d <= '9'))
      uu = ((d - '0') << 4);
    else if ((d >= 'a') && (d <= 'f'))
      uu = ((d - ('a'-10)) << 4);
    d = *(c++);
    if ((d >= '0') && (d <= '9'))
      uu |= (d - '0');
    else if ((d >= 'a') && (d <= 'f'))
      uu |= (d - ('a'-10));
    *u = uu;
  }
  return c;
}

/* Convert a pointer value */
SWIGRUNTIME(int)
SWIG_ConvertPtr(PyObject *obj, void **ptr, swig_type_info *ty, int flags) {
  swig_type_info *tc;
  char  *c;
  static PyObject *SWIG_this = 0;
  int    newref = 0;

  if (!obj) return 0;
  if (obj == Py_None) {
    *ptr = 0;
    return 0;
  }
#ifdef SWIG_COBJECT_TYPES
  if (!(PyCObject_Check(obj))) {
    if (!SWIG_this)
      SWIG_this = PyString_InternFromString("this");
    obj = PyObject_GetAttr(obj,SWIG_this);
    newref = 1;
    if (!obj) goto type_error;
    if (!PyCObject_Check(obj)) {
      Py_DECREF(obj);
      goto type_error;
    }
  } 
  *ptr = PyCObject_AsVoidPtr(obj);
  c = (char *) PyCObject_GetDesc(obj);
  if (newref) Py_DECREF(obj);
  goto cobject;
#else
  if (!(PyString_Check(obj))) {
    if (!SWIG_this)
      SWIG_this = PyString_InternFromString("this");
    obj = PyObject_GetAttr(obj,SWIG_this);
    newref = 1;
    if (!obj) goto type_error;
    if (!PyString_Check(obj)) {
      Py_DECREF(obj);
      goto type_error;
    }
  } 
  c = PyString_AsString(obj);
  /* Pointer values must start with leading underscore */
  if (*c != '_') {
    *ptr = (void *) 0;
    if (strcmp(c,"NULL") == 0) {
      if (newref) Py_DECREF(obj);
      return 0;
    } else {
      if (newref) Py_DECREF(obj);
      goto type_error;
    }
  }
  c++;
  c = SWIG_UnpackData(c,ptr,sizeof(void *));
  if (newref) Py_DECREF(obj);
#endif

#ifdef SWIG_COBJECT_TYPES
cobject:
#endif

  if (ty) {
    tc = SWIG_TypeCheck(c,ty);
    if (!tc) goto type_error;
    *ptr = SWIG_TypeCast(tc,(void*) *ptr);
  }
  return 0;

type_error:
  if (flags) {
    if (ty) {
      char *temp = (char *) malloc(64+strlen(ty->name));
      sprintf(temp,"Type error. Expected %s", ty->name);
      PyErr_SetString(PyExc_TypeError, temp);
      free((char *) temp);
    } else {
      PyErr_SetString(PyExc_TypeError,"Expected a pointer");
    }
  }
  return -1;
}

/* Convert a packed value value */
SWIGRUNTIME(int)
SWIG_ConvertPacked(PyObject *obj, void *ptr, int sz, swig_type_info *ty, int flags) {
  swig_type_info *tc;
  char  *c;

  if ((!obj) || (!PyString_Check(obj))) goto type_error;
  c = PyString_AsString(obj);
  /* Pointer values must start with leading underscore */
  if (*c != '_') goto type_error;
  c++;
  c = SWIG_UnpackData(c,ptr,sz);
  if (ty) {
    tc = SWIG_TypeCheck(c,ty);
    if (!tc) goto type_error;
  }
  return 0;

type_error:

  if (flags) {
    if (ty) {
      char *temp = (char *) malloc(64+strlen(ty->name));
      sprintf(temp,"Type error. Expected %s", ty->name);
      PyErr_SetString(PyExc_TypeError, temp);
      free((char *) temp);
    } else {
      PyErr_SetString(PyExc_TypeError,"Expected a pointer");
    }
  }
  return -1;
}

/* Create a new pointer object */
SWIGRUNTIME(PyObject *)
SWIG_NewPointerObj(void *ptr, swig_type_info *type, int own) {
  PyObject *robj;
  if (!ptr) {
    Py_INCREF(Py_None);
    return Py_None;
  }
#ifdef SWIG_COBJECT_TYPES
  robj = PyCObject_FromVoidPtrAndDesc((void *) ptr, (char *) type->name, NULL);
#else
  {
    char result[512];
    char *r = result;
    *(r++) = '_';
    r = SWIG_PackData(r,&ptr,sizeof(void *));
    strcpy(r,type->name);
    robj = PyString_FromString(result);
  }
#endif
  if (!robj || (robj == Py_None)) return robj;
  if (type->clientdata) {
    PyObject *inst;
    PyObject *args = Py_BuildValue((char*)"(O)", robj);
    Py_DECREF(robj);
    inst = PyObject_CallObject((PyObject *) type->clientdata, args);
    Py_DECREF(args);
    if (own) {
      PyObject *n = PyInt_FromLong(1);
      PyObject_SetAttrString(inst,(char*)"thisown",n);
      Py_DECREF(n);
    }
    robj = inst;
  }
  return robj;
}

SWIGRUNTIME(PyObject *)
SWIG_MakeShadow(PyObject *robj, swig_type_info *type, int own) {
  if (!robj || (robj == Py_None)) return robj;
  if (type->clientdata) {
    PyInstanceObject *inst;
    inst = PyObject_NEW(PyInstanceObject, &PyInstance_Type);
    if (!inst) return robj;
    inst->in_dict = PyDict_New();
    inst->in_class = (PyClassObject *) type->clientdata;
    Py_INCREF(inst->in_class);
    PyObject_SetAttrString((PyObject *)inst,(char*)"this",robj);
    Py_DECREF(robj);
    if (own) {
      PyObject *n = PyInt_FromLong(1);
      PyObject_SetAttrString((PyObject *)inst,(char*)"thisown",n);
      Py_DECREF(n);
    }
    robj = (PyObject *) inst;
    Py_INCREF(robj);
  }
  return robj;
}

SWIGRUNTIME(PyObject *)
SWIG_NewPackedObj(void *ptr, int sz, swig_type_info *type) {
  char result[1024];
  char *r = result;
  if ((2*sz + 1 + strlen(type->name)) > 1000) return 0;
  *(r++) = '_';
  r = SWIG_PackData(r,ptr,sz);
  strcpy(r,type->name);
  return PyString_FromString(result);
}

/* Install Constants */
SWIGRUNTIME(void)
SWIG_InstallConstants(PyObject *d, swig_const_info constants[]) {
  int i;
  PyObject *obj;
  for (i = 0; constants[i].type; i++) {
    switch(constants[i].type) {
    case SWIG_PY_INT:
      obj = PyInt_FromLong(constants[i].lvalue);
      break;
    case SWIG_PY_FLOAT:
      obj = PyFloat_FromDouble(constants[i].dvalue);
      break;
    case SWIG_PY_STRING:
      obj = PyString_FromString((char *) constants[i].pvalue);
      break;
    case SWIG_PY_POINTER:
      obj = SWIG_NewPointerObj(constants[i].pvalue, *(constants[i]).ptype,0);
      break;
    case SWIG_PY_BINARY:
      obj = SWIG_NewPackedObj(constants[i].pvalue, constants[i].lvalue, *(constants[i].ptype));
      break;
    default:
      obj = 0;
      break;
    }
    if (obj) {
      PyDict_SetItemString(d,constants[i].name,obj);
      Py_DECREF(obj);
    }
  }
}

#endif

#ifdef __cplusplus
}
#endif






