/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.11u-20020201-2200
 * 
 * This file is not intended to be easily readable and contains a number of 
 * coding conventions designed to improve portability and efficiency. Do not make
 * changes to this file unless you know what you are doing--modify the SWIG 
 * interface file instead. 
 * ----------------------------------------------------------------------------- */

#define SWIGPYTHON

#ifdef __cplusplus
template<class T> class SwigValueWrapper {
    T *tt;
public:
    inline SwigValueWrapper() : tt(0) { }
    inline ~SwigValueWrapper() { if (tt) delete tt; } 
    inline SwigValueWrapper& operator=(T t) {tt = new T(t); return *this;}
    inline operator T() const {return *tt;}
    inline T *operator&() { return tt; }
};                                                    
#endif

/***********************************************************************
 * common.swg
 *
 *     This file contains generic SWIG runtime support for pointer
 *     type checking as well as a few commonly used macros to control
 *     external linkage.
 *
 * Author : David Beazley (beazley@cs.uchicago.edu)
 *
 * Copyright (c) 1999-2000, The University of Chicago
 * 
 * This file may be freely redistributed without license or fee provided
 * this copyright message remains intact.
 ************************************************************************/

#include <string.h>

#if defined(_WIN32) || defined(__WIN32__)
#       if defined(_MSC_VER)
#               if defined(STATIC_LINKED)
#                       define SWIGEXPORT(a) a
#               else
#                       define SWIGEXPORT(a) __declspec(dllexport) a
#               endif
#       else
#               if defined(__BORLANDC__)
#                       define SWIGEXPORT(a) a _export
#               else
#                       define SWIGEXPORT(a) a
#       endif
#endif
#else
#       define SWIGEXPORT(a) a
#endif

#ifdef SWIG_GLOBAL
#define SWIGRUNTIME(a) SWIGEXPORT(a)
#else
#define SWIGRUNTIME(a) static a
#endif



#ifdef __cplusplus
extern "C" {
#endif

typedef void *(*swig_converter_func)(void *);

typedef struct swig_type_info {
  const char             *name;                 
  swig_converter_func     converter;
  const char             *str;
  struct swig_type_info  *next;
  struct swig_type_info  *prev;
  void                   *clientdata;	
} swig_type_info;

#ifdef SWIG_NOINCLUDE
SWIGEXPORT(swig_type_info *) SWIG_TypeRegister(swig_type_info *);
SWIGEXPORT(swig_type_info *) SWIG_TypeCheck(char *c, swig_type_info *);
SWIGEXPORT(void *) SWIG_TypeCast(swig_type_info *, void *);
SWIGEXPORT(swig_type_info *) SWIG_TypeQuery(const char *);
SWIGEXPORT(void) SWIG_TypeClientData(swig_type_info *, void *);
#else

static swig_type_info *swig_type_list = 0;

/* Register a type mapping with the type-checking */
SWIGRUNTIME(swig_type_info *)
SWIG_TypeRegister(swig_type_info *ti)
{
  swig_type_info *tc, *head, *ret, *next;
  /* Check to see if this type has already been registered */
  tc = swig_type_list;
  while (tc) {
    if (strcmp(tc->name, ti->name) == 0) {
      /* Already exists in the table.  Just add additional types to the list */
      head = tc;
      next = tc->next;
      goto l1;
    }
    tc = tc->prev;
  }
  head = ti;
  next = 0;

  /* Place in list */
  ti->prev = swig_type_list;
  swig_type_list = ti;

  /* Build linked lists */
 l1:
  ret = head;
  tc = ti + 1;
  /* Patch up the rest of the links */
  while (tc->name) {
    head->next = tc;
    tc->prev = head;
    head = tc;
    tc++;
  }
  head->next = next;
  return ret;
}

/* Check the typename */
SWIGRUNTIME(swig_type_info *) 
SWIG_TypeCheck(char *c, swig_type_info *ty)
{
  swig_type_info *s;
  if (!ty) return 0;        /* Void pointer */
  s = ty->next;             /* First element always just a name */
  while (s) {
    if (strcmp(s->name,c) == 0) {
      if (s == ty->next) return s;
      /* Move s to the top of the linked list */
      s->prev->next = s->next;
      if (s->next) {
	s->next->prev = s->prev;
      }
      /* Insert s as second element in the list */
      s->next = ty->next;
      if (ty->next) ty->next->prev = s;
      ty->next = s;
      return s;
    }
    s = s->next;
  }
  return 0;
}

/* Cast a pointer (needed for C++ inheritance */
SWIGRUNTIME(void *) 
SWIG_TypeCast(swig_type_info *ty, void *ptr) 
{
  if ((!ty) || (!ty->converter)) return ptr;
  return (*ty->converter)(ptr);
}

/* Search for a swig_type_info structure */
SWIGRUNTIME(swig_type_info *)
SWIG_TypeQuery(const char *name) {
  swig_type_info *ty = swig_type_list;
  while (ty) {
    if (ty->str && (strcmp(name,ty->str) == 0)) return ty;
    if (ty->name && (strcmp(name,ty->name) == 0)) return ty;
    ty = ty->prev;
  }
  return 0;
}

/* Set the clientdata field for a type */
SWIGRUNTIME(void)
SWIG_TypeClientData(swig_type_info *ti, void *clientdata) {
  swig_type_info *tc, *equiv;
  if (ti->clientdata) return;
  ti->clientdata = clientdata;
  equiv = ti->next;
  while (equiv) {
    if (!equiv->converter) {
      tc = swig_type_list;
      while (tc) {
	if ((strcmp(tc->name, equiv->name) == 0))
	  SWIG_TypeClientData(tc,clientdata);
	tc = tc->prev;
      }
    }
    equiv = equiv->next;
  }
}
#endif

#ifdef __cplusplus
}

#endif




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








/* -------- TYPES TABLE (BEGIN) -------- */

#define  SWIGTYPE_p_World swig_types[0] 
#define  SWIGTYPE_p_Actor swig_types[1] 
#define  SWIGTYPE_p_Matrix swig_types[2] 
static swig_type_info *swig_types[4];

/* -------- TYPES TABLE (END) -------- */

#define SWIG_init    initgame

#define SWIG_name    "game"

	/* Put header files here (optional) */

#include "..\..\shared\Shared.h"
#include "Engine.h"
#include "ScriptInterface.h"

	
#ifdef __cplusplus
extern "C" {
#endif
static PyObject *_wrap_SpawnActor(PyObject *self, PyObject *args) {
    PyObject *resultobj;
    char *arg1 ;
    World *arg2 ;
    Matrix *arg3 ;
    Actor *result;
    PyObject * obj1  = 0 ;
    PyObject * obj2  = 0 ;
    
    if(!PyArg_ParseTuple(args,(char *)"sOO:SpawnActor",&arg1,&obj1,&obj2)) return NULL;
    if ((SWIG_ConvertPtr(obj1,(void **) &arg2, SWIGTYPE_p_World,1)) == -1) return NULL;
    if ((SWIG_ConvertPtr(obj2,(void **) &arg3, SWIGTYPE_p_Matrix,1)) == -1) return NULL;
    result = (Actor *)SpawnActor(arg1,arg2,arg3);
    
    resultobj = SWIG_NewPointerObj((void *) result, SWIGTYPE_p_Actor, 0);
    return resultobj;
}


static PyObject *_wrap_GetDayTimeMinutes(PyObject *self, PyObject *args) {
    PyObject *resultobj;
    float result;
    
    if(!PyArg_ParseTuple(args,(char *)":GetDayTimeMinutes")) return NULL;
    result = (float )GetDayTimeMinutes();
    
    resultobj = PyFloat_FromDouble(result);
    return resultobj;
}


static PyObject *_wrap_GetDayTimeSpeed(PyObject *self, PyObject *args) {
    PyObject *resultobj;
    float result;
    
    if(!PyArg_ParseTuple(args,(char *)":GetDayTimeSpeed")) return NULL;
    result = (float )GetDayTimeSpeed();
    
    resultobj = PyFloat_FromDouble(result);
    return resultobj;
}


static PyObject *_wrap_FadeSky(PyObject *self, PyObject *args) {
    PyObject *resultobj;
    int arg1 ;
    float arg2 ;
    float arg3 ;
    
    if(!PyArg_ParseTuple(args,(char *)"iff:FadeSky",&arg1,&arg2,&arg3)) return NULL;
    FadeSky(arg1,arg2,arg3);
    
    Py_INCREF(Py_None); resultobj = Py_None;
    return resultobj;
}


static PyObject *_wrap_GetSkyValue(PyObject *self, PyObject *args) {
    PyObject *resultobj;
    int arg1 ;
    float result;
    
    if(!PyArg_ParseTuple(args,(char *)"i:GetSkyValue",&arg1)) return NULL;
    result = (float )GetSkyValue(arg1);
    
    resultobj = PyFloat_FromDouble(result);
    return resultobj;
}


static PyObject *_wrap_FadeFog(PyObject *self, PyObject *args) {
    PyObject *resultobj;
    float arg1 ;
    float arg2 ;
    float arg3 ;
    float arg4 ;
    float arg5 ;
    
    if(!PyArg_ParseTuple(args,(char *)"fffff:FadeFog",&arg1,&arg2,&arg3,&arg4,&arg5)) return NULL;
    FadeFog(arg1,arg2,arg3,arg4,arg5);
    
    Py_INCREF(Py_None); resultobj = Py_None;
    return resultobj;
}


static PyObject *_wrap_SetSkyState(PyObject *self, PyObject *args) {
    PyObject *resultobj;
    int arg1 ;
    
    if(!PyArg_ParseTuple(args,(char *)"i:SetSkyState",&arg1)) return NULL;
    SetSkyState(arg1);
    
    Py_INCREF(Py_None); resultobj = Py_None;
    return resultobj;
}


static PyObject *_wrap_GetSkyState(PyObject *self, PyObject *args) {
    PyObject *resultobj;
    int result;
    
    if(!PyArg_ParseTuple(args,(char *)":GetSkyState")) return NULL;
    result = (int )GetSkyState();
    
    resultobj = PyInt_FromLong((long)result);
    return resultobj;
}


static PyObject *_wrap_SetWeatherState(PyObject *self, PyObject *args) {
    PyObject *resultobj;
    int arg1 ;
    
    if(!PyArg_ParseTuple(args,(char *)"i:SetWeatherState",&arg1)) return NULL;
    SetWeatherState(arg1);
    
    Py_INCREF(Py_None); resultobj = Py_None;
    return resultobj;
}


static PyObject *_wrap_GetWeatherState(PyObject *self, PyObject *args) {
    PyObject *resultobj;
    int result;
    
    if(!PyArg_ParseTuple(args,(char *)":GetWeatherState")) return NULL;
    result = (int )GetWeatherState();
    
    resultobj = PyInt_FromLong((long)result);
    return resultobj;
}


static PyObject *_wrap_SetSkyDayTime(PyObject *self, PyObject *args) {
    PyObject *resultobj;
    float arg1 ;
    
    if(!PyArg_ParseTuple(args,(char *)"f:SetSkyDayTime",&arg1)) return NULL;
    SetSkyDayTime(arg1);
    
    Py_INCREF(Py_None); resultobj = Py_None;
    return resultobj;
}


static PyObject *_wrap_DoMessageBox(PyObject *self, PyObject *args) {
    PyObject *resultobj;
    char *arg1 ;
    
    if(!PyArg_ParseTuple(args,(char *)"s:DoMessageBox",&arg1)) return NULL;
    DoMessageBox(arg1);
    
    Py_INCREF(Py_None); resultobj = Py_None;
    return resultobj;
}


static PyObject *_wrap_Print(PyObject *self, PyObject *args) {
    PyObject *resultobj;
    char *arg1 ;
    
    if(!PyArg_ParseTuple(args,(char *)"s:Print",&arg1)) return NULL;
    Print(arg1);
    
    Py_INCREF(Py_None); resultobj = Py_None;
    return resultobj;
}


static PyObject *_wrap_getDeltaTime(PyObject *self, PyObject *args) {
    PyObject *resultobj;
    float result;
    
    if(!PyArg_ParseTuple(args,(char *)":getDeltaTime")) return NULL;
    result = (float )getDeltaTime();
    
    resultobj = PyFloat_FromDouble(result);
    return resultobj;
}


static PyObject *_wrap_getRandomWholeNumber(PyObject *self, PyObject *args) {
    PyObject *resultobj;
    int arg1 ;
    int result;
    
    if(!PyArg_ParseTuple(args,(char *)"i:getRandomWholeNumber",&arg1)) return NULL;
    result = (int )getRandomWholeNumber(arg1);
    
    resultobj = PyInt_FromLong((long)result);
    return resultobj;
}


static PyObject *_wrap_addInteriorVolumeBox(PyObject *self, PyObject *args) {
    PyObject *resultobj;
    Actor *arg1 ;
    PyObject * obj0  = 0 ;
    
    if(!PyArg_ParseTuple(args,(char *)"O:addInteriorVolumeBox",&obj0)) return NULL;
    if ((SWIG_ConvertPtr(obj0,(void **) &arg1, SWIGTYPE_p_Actor,1)) == -1) return NULL;
    addInteriorVolumeBox(arg1);
    
    Py_INCREF(Py_None); resultobj = Py_None;
    return resultobj;
}


static PyObject *_wrap_PlayOgg(PyObject *self, PyObject *args) {
    PyObject *resultobj;
    char *arg1 ;
    
    if(!PyArg_ParseTuple(args,(char *)"s:PlayOgg",&arg1)) return NULL;
    PlayOgg(arg1);
    
    Py_INCREF(Py_None); resultobj = Py_None;
    return resultobj;
}


static PyMethodDef SwigMethods[] = {
	 { (char *)"SpawnActor", _wrap_SpawnActor, METH_VARARGS },
	 { (char *)"GetDayTimeMinutes", _wrap_GetDayTimeMinutes, METH_VARARGS },
	 { (char *)"GetDayTimeSpeed", _wrap_GetDayTimeSpeed, METH_VARARGS },
	 { (char *)"FadeSky", _wrap_FadeSky, METH_VARARGS },
	 { (char *)"GetSkyValue", _wrap_GetSkyValue, METH_VARARGS },
	 { (char *)"FadeFog", _wrap_FadeFog, METH_VARARGS },
	 { (char *)"SetSkyState", _wrap_SetSkyState, METH_VARARGS },
	 { (char *)"GetSkyState", _wrap_GetSkyState, METH_VARARGS },
	 { (char *)"SetWeatherState", _wrap_SetWeatherState, METH_VARARGS },
	 { (char *)"GetWeatherState", _wrap_GetWeatherState, METH_VARARGS },
	 { (char *)"SetSkyDayTime", _wrap_SetSkyDayTime, METH_VARARGS },
	 { (char *)"DoMessageBox", _wrap_DoMessageBox, METH_VARARGS },
	 { (char *)"Print", _wrap_Print, METH_VARARGS },
	 { (char *)"getDeltaTime", _wrap_getDeltaTime, METH_VARARGS },
	 { (char *)"getRandomWholeNumber", _wrap_getRandomWholeNumber, METH_VARARGS },
	 { (char *)"addInteriorVolumeBox", _wrap_addInteriorVolumeBox, METH_VARARGS },
	 { (char *)"PlayOgg", _wrap_PlayOgg, METH_VARARGS },
	 { NULL, NULL }
};


/* -------- TYPE CONVERSION AND EQUIVALENCE RULES (BEGIN) -------- */

static swig_type_info _swigt__p_World[] = {{"_p_World", 0, "World *"},{"_p_World"},{0}};
static swig_type_info _swigt__p_Actor[] = {{"_p_Actor", 0, "Actor *"},{"_p_Actor"},{0}};
static swig_type_info _swigt__p_Matrix[] = {{"_p_Matrix", 0, "Matrix *"},{"_p_Matrix"},{0}};

static swig_type_info *swig_types_initial[] = {
_swigt__p_World, 
_swigt__p_Actor, 
_swigt__p_Matrix, 
0
};


/* -------- TYPE CONVERSION AND EQUIVALENCE RULES (END) -------- */

static swig_const_info swig_const_table[] = {
{ SWIG_PY_INT,     (char *)"SKY_MOON_INTENSITY", (long) 4, 0, 0, 0},
{ SWIG_PY_INT,     (char *)"SKY_MOON_SIZE", (long) 5, 0, 0, 0},
{ SWIG_PY_INT,     (char *)"SKY_SUN_INTENSITY", (long) 6, 0, 0, 0},
{ SWIG_PY_INT,     (char *)"SKY_SUN_SIZE", (long) 7, 0, 0, 0},
{ SWIG_PY_INT,     (char *)"CLOUD_SCROLL_SPEED", (long) 8, 0, 0, 0},
{ SWIG_PY_INT,     (char *)"CLOUD_BRIGHTNESS_FACTOR", (long) 9, 0, 0, 0},
{ SWIG_PY_INT,     (char *)"CLOUD_SOLIDITY", (long) 10, 0, 0, 0},
{ SWIG_PY_INT,     (char *)"CLOUD_MAX_INTENSITY", (long) 11, 0, 0, 0},
{ SWIG_PY_INT,     (char *)"CLOUD_MIN_INTENSITY", (long) 12, 0, 0, 0},
{ SWIG_PY_INT,     (char *)"CLOUD_ADD_RED_VALUE", (long) 13, 0, 0, 0},
{ SWIG_PY_INT,     (char *)"CLOUD_ADD_GREEN_VALUE", (long) 14, 0, 0, 0},
{ SWIG_PY_INT,     (char *)"CLOUD_ADD_BLUE_VALUE", (long) 15, 0, 0, 0},
{ SWIG_PY_INT,     (char *)"CLOUD_ALPHA_FACTOR", (long) 16, 0, 0, 0},
{ SWIG_PY_INT,     (char *)"DAYLIGHT_RED", (long) 17, 0, 0, 0},
{ SWIG_PY_INT,     (char *)"DAYLIGHT_GREEN", (long) 18, 0, 0, 0},
{ SWIG_PY_INT,     (char *)"DAYLIGHT_BLUE", (long) 19, 0, 0, 0},
{ SWIG_PY_INT,     (char *)"NIGHTLIGHT_RED", (long) 20, 0, 0, 0},
{ SWIG_PY_INT,     (char *)"NIGHTLIGHT_GREEN", (long) 21, 0, 0, 0},
{ SWIG_PY_INT,     (char *)"NIGHTLIGHT_BLUE", (long) 22, 0, 0, 0},
{ SWIG_PY_INT,     (char *)"RAIN_INTENSITY", (long) 23, 0, 0, 0},
{ SWIG_PY_INT,     (char *)"SKY_ADD_RED_VALUE", (long) 24, 0, 0, 0},
{ SWIG_PY_INT,     (char *)"SKY_ADD_GREEN_VALUE", (long) 25, 0, 0, 0},
{ SWIG_PY_INT,     (char *)"SKY_ADD_BLUE_VALUE", (long) 26, 0, 0, 0},
{ SWIG_PY_INT,     (char *)"SKY_OVERBRIGHT", (long) 27, 0, 0, 0},
{ SWIG_PY_INT,     (char *)"SKY_BGMIX1", (long) 28, 0, 0, 0},
{ SWIG_PY_INT,     (char *)"SKY_BGMIX2", (long) 29, 0, 0, 0},
{ SWIG_PY_INT,     (char *)"SKY_BGMIX3", (long) 30, 0, 0, 0},
{ SWIG_PY_INT,     (char *)"SKY_BGMIX4", (long) 31, 0, 0, 0},
{ SWIG_PY_INT,     (char *)"SKY_BGMIX5", (long) 32, 0, 0, 0},
{ SWIG_PY_INT,     (char *)"SKY_BGMIX6", (long) 33, 0, 0, 0},
{ SWIG_PY_INT,     (char *)"SKY_BGMIX7", (long) 34, 0, 0, 0},
{ SWIG_PY_INT,     (char *)"SKY_BGMIX8", (long) 35, 0, 0, 0},
{ SWIG_PY_INT,     (char *)"WEATHERSTATE_CLEAR", (long) 0, 0, 0, 0},
{ SWIG_PY_INT,     (char *)"WEATHERSTATE_THUNDERSTORM", (long) 1, 0, 0, 0},
{0}};

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C"
#endif
SWIGEXPORT(void) SWIG_init(void) {
    static PyObject *SWIG_globals = 0; 
    static int       typeinit = 0;
    PyObject *m, *d;
    int       i;
    if (!SWIG_globals) SWIG_globals = SWIG_newvarlink();
    m = Py_InitModule((char *) SWIG_name, SwigMethods);
    d = PyModule_GetDict(m);
    
    if (!typeinit) {
        for (i = 0; swig_types_initial[i]; i++) {
            swig_types[i] = SWIG_TypeRegister(swig_types_initial[i]);
        }
        typeinit = 1;
    }
    SWIG_InstallConstants(d,swig_const_table);
    
}

