//
// SWIG pointer conversion and utility library
// 
// Dave Beazley
// April 19, 1997
//
// Tcl specific implementation.   This file is included
// by the file ../pointer.i


%{
#include <ctype.h>

/* Types used by the library */
static swig_type_info *SWIG_POINTER_int_p = 0;
static swig_type_info *SWIG_POINTER_short_p =0;
static swig_type_info *SWIG_POINTER_long_p = 0;
static swig_type_info *SWIG_POINTER_float_p = 0;
static swig_type_info *SWIG_POINTER_double_p = 0;
static swig_type_info *SWIG_POINTER_char_p = 0;
static swig_type_info *SWIG_POINTER_char_pp = 0;
%}

%init %{
  SWIG_POINTER_int_p = SWIG_TypeQuery("int *");
  SWIG_POINTER_short_p = SWIG_TypeQuery("short *");
  SWIG_POINTER_long_p = SWIG_TypeQuery("long *");
  SWIG_POINTER_float_p = SWIG_TypeQuery("float *");
  SWIG_POINTER_double_p = SWIG_TypeQuery("double *");
  SWIG_POINTER_char_p = SWIG_TypeQuery("char *");
  SWIG_POINTER_char_pp = SWIG_TypeQuery("char **");
%}

%{

/*------------------------------------------------------------------
  ptrvalue(ptr,type = 0)

  Attempts to dereference a pointer value.  If type is given, it 
  will try to use that type.  Otherwise, this function will attempt
  to "guess" the proper datatype by checking against all of the 
  builtin C datatypes. 
  ------------------------------------------------------------------ */

static int ptrvalue(Tcl_Interp *interp, char *ptrvalue, int index, const char *type) {
  void     *ptr;
  char     *s;
  int      error = 0;

  if (type) {
    if (strlen(type) == 0) type = 0;
  }
  s = ptrvalue;
  if (SWIG_ConvertPtrFromString(interp,s,&ptr,0) != TCL_OK) {
    Tcl_SetResult(interp, (char *) "Type error in ptrvalue. Argument is not a valid pointer value.", TCL_STATIC);
    return TCL_ERROR;
  }

  /* If no datatype was passed, try a few common datatypes first */
  if (!type) {
    /* No datatype was passed.   Type to figure out if it's a common one */
    if (SWIG_ConvertPtrFromString(interp,s,&ptr,SWIG_POINTER_int_p) == TCL_OK) {
      type = "int";
    } else if (SWIG_ConvertPtrFromString(interp,s,&ptr,SWIG_POINTER_double_p) == TCL_OK) {
      type = "double";
    } else if (SWIG_ConvertPtrFromString(interp,s,&ptr,SWIG_POINTER_short_p) == TCL_OK) {
      type = "short";
    } else if (SWIG_ConvertPtrFromString(interp,s,&ptr,SWIG_POINTER_long_p) == TCL_OK) {
      type = "long";
    } else if (SWIG_ConvertPtrFromString(interp,s,&ptr,SWIG_POINTER_float_p) == TCL_OK) {
      type = "float";
    } else if (SWIG_ConvertPtrFromString(interp,s,&ptr,SWIG_POINTER_char_p) == TCL_OK) {
      type = "char";
    } else if (SWIG_ConvertPtrFromString(interp,s,&ptr,SWIG_POINTER_char_pp) == TCL_OK) {
      type = "char *";
    } else {
      type = "unknown";
    }
  }

  if (!ptr) {
    Tcl_SetResult(interp, (char *) "Unable to dereference NULL pointer.",TCL_STATIC);
    return TCL_ERROR;
  }

  /* Now we have a datatype.  Try to figure out what to do about it */
  if (strcmp(type,"int") == 0) {
    sprintf(interp->result,"%ld",(long) *(((int *) ptr) + index));
  } else if (strcmp(type,"double") == 0) {
    Tcl_PrintDouble(interp,(double) *(((double *) ptr)+index), interp->result);
  } else if (strcmp(type,"short") == 0) {
    sprintf(interp->result,"%ld",(long) *(((short *) ptr) + index));
  } else if (strcmp(type,"long") == 0) {
    sprintf(interp->result,"%ld",(long) *(((long *) ptr) + index));
  } else if (strcmp(type,"float") == 0) {
    Tcl_PrintDouble(interp,(double) *(((float *) ptr)+index), interp->result);
  } else if (strcmp(type,"char") == 0) {
    Tcl_SetResult(interp,((char *) ptr) + index, TCL_VOLATILE);
  } else if (strcmp(type,"char *") == 0) {
    char *c = *(((char **) ptr)+index);
    if (c) Tcl_SetResult(interp,(char *) c, TCL_VOLATILE);
    else Tcl_SetResult(interp, (char *) "NULL", TCL_VOLATILE);
  } else {
    Tcl_SetResult(interp, (char *) "Unable to dereference unsupported datatype.",TCL_STATIC);
    return TCL_ERROR;
  }
  return TCL_OK;
}

/*------------------------------------------------------------------
  ptrcreate(type,value = 0,numelements = 1)

  Attempts to create a new object of given type.  Type must be
  a basic C datatype.  Will not create complex objects.
  ------------------------------------------------------------------ */

static int ptrcreate(Tcl_Interp *interp, char *type, char *ptrvalue, int numelements) {
  void     *ptr;
  int       sz;
  swig_type_info *cast = 0;
  char      temp[40];

  /* Check the type string against a variety of possibilities */

  if (strcmp(type,"int") == 0) {
    sz = sizeof(int)*numelements;
    cast = SWIG_POINTER_int_p;
  } else if (strcmp(type,"short") == 0) {
    sz = sizeof(short)*numelements;
    cast = SWIG_POINTER_short_p;
  } else if (strcmp(type,"long") == 0) {
    sz = sizeof(long)*numelements;
    cast = SWIG_POINTER_long_p;
  } else if (strcmp(type,"double") == 0) {
    sz = sizeof(double)*numelements;
    cast = SWIG_POINTER_double_p;
  } else if (strcmp(type,"float") == 0) {
    sz = sizeof(float)*numelements;
    cast = SWIG_POINTER_float_p;
  } else if (strcmp(type,"char") == 0) {
    sz = sizeof(char)*numelements;
    cast = SWIG_POINTER_char_p;
  } else if (strcmp(type,"char *") == 0) {
    sz = sizeof(char *)*(numelements+1);
    cast = SWIG_POINTER_char_pp;
  } else if (strcmp(type,"void") == 0) {
    sz = numelements;
  } else {
    Tcl_SetResult(interp, (char *) "Unable to create unknown datatype.",TCL_STATIC);
    return TCL_ERROR;
  }
   
  /* Create the new object */
  
  ptr = (void *) calloc(1,sz);
  if (!ptr) {
    Tcl_SetResult(interp, (char *) "Out of memory in ptrcreate.",TCL_STATIC); 
    return TCL_ERROR;
  }

  /* Now try to set its default value */

  if (ptrvalue) {
    if (strcmp(type,"int") == 0) {
      int *ip,i,ivalue;
      Tcl_GetInt(interp,ptrvalue,&ivalue);
      ip = (int *) ptr;
      for (i = 0; i < numelements; i++)
	ip[i] = ivalue;
    } else if (strcmp(type,"short") == 0) {
      short *ip;
      int i, ivalue;
      Tcl_GetInt(interp,ptrvalue,&ivalue);      
      ip = (short *) ptr;
      for (i = 0; i < numelements; i++)
	ip[i] = (short) ivalue;
    } else if (strcmp(type,"long") == 0) {
      long *ip;
      int i, ivalue;
      Tcl_GetInt(interp,ptrvalue,&ivalue);      
      ip = (long *) ptr;
      for (i = 0; i < numelements; i++)
	ip[i] = (long) ivalue;
    } else if (strcmp(type,"double") == 0) {
      double *ip,ivalue;
      int i;
      Tcl_GetDouble(interp,ptrvalue,&ivalue);
      ip = (double *) ptr;
      for (i = 0; i < numelements; i++)
	ip[i] = ivalue;
    } else if (strcmp(type,"float") == 0) {
      float *ip;
      double ivalue;
      int i;
      Tcl_GetDouble(interp,ptrvalue,&ivalue);
      ip = (float *) ptr;
      for (i = 0; i < numelements; i++)
	ip[i] = (double) ivalue;
    } else if (strcmp(type,"char") == 0) {
      char *ip,*ivalue;
      ivalue = (char *) ptrvalue;
      ip = (char *) ptr;
      strncpy(ip,ivalue,numelements-1);
    } else if (strcmp(type,"char *") == 0) {
      char **ip, *ivalue;
      int  i;
      ivalue = (char *) ptrvalue;
      ip = (char **) ptr;
      for (i = 0; i < numelements; i++) {
	if (ivalue) {
	  ip[i] = (char *) malloc(strlen(ivalue)+1);
	  strcpy(ip[i],ivalue);
	} else {
	  ip[i] = 0;
	}
      }
      ip[numelements] = 0;
    } 
  } 
  /* Create the pointer value */

  Tcl_SetObjResult(interp,SWIG_NewPointerObj(ptr,cast));
  return TCL_OK;
}

/*------------------------------------------------------------------
  ptrset(ptr,value,index = 0,type = 0)

  Attempts to set the value of a pointer variable.  If type is
  given, we will use that type.  Otherwise, we'll guess the datatype.
  ------------------------------------------------------------------ */

static int ptrset(Tcl_Interp *interp, char *ptrvalue, char *value, int index, const char *type) {
  void     *ptr;
  char     *s;

  s = ptrvalue;
  if (SWIG_ConvertPtrFromString(interp,s,&ptr,0) != TCL_OK) {
    Tcl_SetResult(interp, (char *) "Type error in ptrset. Argument is not a valid pointer value.",
		  TCL_STATIC);
    return TCL_ERROR;
  }

  /* If no datatype was passed, try a few common datatypes first */

  if (!type) {

    /* No datatype was passed.   Type to figure out if it's a common one */
    if (SWIG_ConvertPtrFromString(interp,s,&ptr,SWIG_POINTER_int_p) == TCL_OK) {
      type = "int";
    } else if (SWIG_ConvertPtrFromString(interp,s,&ptr,SWIG_POINTER_double_p) == TCL_OK) {
      type = "double";
    } else if (SWIG_ConvertPtrFromString(interp,s,&ptr,SWIG_POINTER_short_p) == TCL_OK) {
      type = "short";
    } else if (SWIG_ConvertPtrFromString(interp,s,&ptr,SWIG_POINTER_long_p) == TCL_OK) {
      type = "long";
    } else if (SWIG_ConvertPtrFromString(interp,s,&ptr,SWIG_POINTER_float_p) == TCL_OK) {
      type = "float";
    } else if (SWIG_ConvertPtrFromString(interp,s,&ptr,SWIG_POINTER_char_p) == TCL_OK) {
      type = "char";
    } else if (SWIG_ConvertPtrFromString(interp,s,&ptr,SWIG_POINTER_char_pp) == TCL_OK) {
      type = "char *";
    } else {
      type = "unknown";
    }
  }

  if (!ptr) {
    Tcl_SetResult(interp, (char *) "Unable to set NULL pointer.",TCL_STATIC);
    return TCL_ERROR;
  }
  
  /* Now we have a datatype.  Try to figure out what to do about it */
  if (strcmp(type,"int") == 0) {
    int ivalue;
    Tcl_GetInt(interp,value, &ivalue);
    *(((int *) ptr)+index) = ivalue;
  } else if (strcmp(type,"double") == 0) {
    double ivalue;
    Tcl_GetDouble(interp,value, &ivalue);
    *(((double *) ptr)+index) = (double) ivalue;
  } else if (strcmp(type,"short") == 0) {
    int ivalue;
    Tcl_GetInt(interp,value, &ivalue);
    *(((short *) ptr)+index) = (short) ivalue;
  } else if (strcmp(type,"long") == 0) {
    int ivalue;
    Tcl_GetInt(interp,value, &ivalue);
    *(((long *) ptr)+index) = (long) ivalue;
  } else if (strcmp(type,"float") == 0) {
    double ivalue;
    Tcl_GetDouble(interp,value, &ivalue);
    *(((float *) ptr)+index) = (float) ivalue;
  } else if (strcmp(type,"char") == 0) {
    char *c = value;
    strcpy(((char *) ptr)+index, c);
  } else if (strcmp(type,"char *") == 0) {
    char *c = value;
    char **ca = (char **) ptr;
    if (ca[index]) free(ca[index]);
    if (strcmp(c,"NULL") == 0) {
      ca[index] = 0;
    } else {
      ca[index] = (char *) malloc(strlen(c)+1);
      strcpy(ca[index],c);
    }
  } else {
    Tcl_SetResult(interp, (char *) "Unable to set unsupported datatype.",TCL_STATIC);
    return TCL_ERROR;
  }
  return TCL_OK;
}

/*------------------------------------------------------------------
  ptradd(ptr,offset)

  Adds a value to an existing pointer value.  Will do a type-dependent
  add for basic datatypes.  For other datatypes, will do a byte-add.
  ------------------------------------------------------------------ */

static int ptradd(Tcl_Interp *interp, char *ptrvalue, int offset) {

  char *r,*s;
  void *ptr,*junk;
  swig_type_info *type = 0;
  swig_type_info stype;

  /* Check to see what kind of object ptrvalue is */
  
  s = ptrvalue;

  /* Try to handle a few common datatypes first */

  if (SWIG_ConvertPtrFromString(interp,s,&ptr,SWIG_POINTER_int_p) == TCL_OK) {
    ptr = (void *) (((int *) ptr) + offset);
    type = SWIG_POINTER_int_p;
  } else if (SWIG_ConvertPtrFromString(interp,s,&ptr,SWIG_POINTER_double_p) == TCL_OK) {
    ptr = (void *) (((double *) ptr) + offset);
    type = SWIG_POINTER_double_p;
  } else if (SWIG_ConvertPtrFromString(interp,s,&ptr,SWIG_POINTER_short_p) == TCL_OK) {
    ptr = (void *) (((short *) ptr) + offset);
    type = SWIG_POINTER_short_p;
  } else if (SWIG_ConvertPtrFromString(interp,s,&ptr,SWIG_POINTER_long_p) == TCL_OK) {
    ptr = (void *) (((long *) ptr) + offset);
    type = SWIG_POINTER_long_p;
  } else if (SWIG_ConvertPtrFromString(interp,s,&ptr,SWIG_POINTER_float_p) == TCL_OK) { 
    ptr = (void *) (((float *) ptr) + offset);
    type = SWIG_POINTER_float_p;
  } else if (SWIG_ConvertPtrFromString(interp,s,&ptr,SWIG_POINTER_char_p) == TCL_OK) {
    ptr = (void *) (((char *) ptr) + offset);
    type = SWIG_POINTER_char_p;
  } else if (SWIG_ConvertPtrFromString(interp,s,&ptr,0) == TCL_OK) {
    ptr = (void *) (((char *) ptr) + offset);
    stype.name = SWIG_PointerTypeFromString(s);
    type = &stype;
  } else {
    Tcl_SetResult(interp, (char *) "Type error in ptradd. Argument is not a valid pointer value.",TCL_STATIC);
    return TCL_ERROR;
  }
  Tcl_SetObjResult(interp,SWIG_NewPointerObj(ptr,type));
  return TCL_OK;
}

/*------------------------------------------------------------------
  ptrfree(ptr)

  Destroys a pointer value
  ------------------------------------------------------------------ */

int ptrfree(Tcl_Interp *interp, char *ptrvalue) {
  void *ptr, *junk;
  char *s;

  s = ptrvalue;
  if (SWIG_ConvertPtrFromString(interp,ptrvalue,&ptr,0) != TCL_OK) {
    Tcl_SetResult(interp, (char *) "Type error in ptrfree. Argument is not a valid pointer value.",TCL_STATIC);
    return TCL_ERROR;
  }

  /* Check to see if this pointer is a char ** */
  if (SWIG_ConvertPtrFromString(interp,ptrvalue,&junk,SWIG_POINTER_char_pp) == TCL_OK) {
    char **c = (char **) ptr;
    if (c) {
      int i = 0;
      while (c[i]) {
	free(c[i]);
	i++;
      }
    }
  } 
  if (ptr)
    free((char *) ptr);
  return TCL_OK;
}
%}

%typemap(tcl8,out) int ptrcast,
                  int ptrvalue,
                  int ptrcreate,
                  int ptrset,
                  int ptradd,
                  int ptrfree  
{
  if ($1 != TCL_OK) return $1;
}

%typemap(tcl8,ignore) Tcl_Interp * {
  $1 = interp;
}

int ptrvalue(Tcl_Interp *interp, char *ptr, int index = 0, const char *type = 0);
// Returns the value that a pointer is pointing to (ie. dereferencing).
// The type is automatically inferred by the pointer type--thus, an
// integer pointer will return an integer, a double will return a double,
// and so on.   The index and type fields are optional parameters.  When
// an index is specified, this function returns the value of ptr[index].
// This allows array access.   When a type is specified, it overrides
// the given pointer type.   Examples :
//
//    ptrvalue $a             #  Returns the value *a
//    ptrvalue $a 10          #  Returns the value a[10]
//    ptrvalue $a 10 double   #  Returns a[10] assuming a is a double *

int ptrset(Tcl_Interp *interp, char *ptr, char *value, int index = 0, const char *type = 0);
// Sets the value pointed to by a pointer.  The type is automatically
// inferred from the pointer type so this function will work for
// integers, floats, doubles, etc...  The index and type fields are
// optional.  When an index is given, it provides array access.  When
// type is specified, it overrides the given pointer type.  Examples :
// 
//   ptrset $a 3             # Sets the value *a = 3
//   ptrset $a 3 10          # Sets a[10] = 3
//   ptrset $a 3 10 int      # Sets a[10] = 3 assuming a is a int *

int ptrcreate(Tcl_Interp *interp, char *type, char *value = 0, int nitems = 1);
// Creates a new object and returns a pointer to it.  This function 
// can be used to create various kinds of objects for use in C functions.
// type specifies the basic C datatype to create and value is an
// optional parameter that can be used to set the initial value of the
// object.  nitems is an optional parameter that can be used to create
// an array.  This function results in a memory allocation using
// malloc().  Examples :
//
//   set a [ptrcreate "double"]    # Create a new double, return pointer
//   set a [ptrcreate int 7]       # Create an integer, set value to 7
//   set a [ptrcreate int 0 1000]  # Create an integer array with initial
//                                 # values all set to zero
//
// This function only recognizes a few common C datatypes as listed below :
//
//        int, short, long, float, double, char, char *, void
//
// All other datatypes will result in an error.  However, other
// datatypes can be created by using the ptrcast function.  For
// example:
//
//  set a [ptrcast [ptrcreate int 0 100],"unsigned int *"]

int ptrfree(Tcl_Interp *interp, char *ptr);
// Destroys the memory pointed to by ptr.  This function calls free()
// and should only be used with objects created by ptrcreate().  Since
// this function calls free, it may work with other objects, but this
// is generally discouraged unless you absolutely know what you're
// doing.

int ptradd(Tcl_Interp *interp, char *ptr, int offset);
// Adds a value to the current pointer value.  For the C datatypes of
// int, short, long, float, double, and char, the offset value is the
// number of objects and works in exactly the same manner as in C.  For
// example, the following code steps through the elements of an array
//
//  set a [ptrcreate double 0 100]       # Create an array double a[100]
//  set b $a
//  for {set i 0} {$i < 100} {incr i 1} {
//      ptrset $b [expr{0.0025*$i}]      # set *b = 0.0025*i
//      set b [ptradd $b 1]              # b++ (go to next double)
//  }
//
// In this case, adding one to b goes to the next double.
// 
// For all other datatypes (including all complex datatypes), the
// offset corresponds to bytes.  This function does not perform any
// bounds checking and negative offsets are perfectly legal.  

    
  

    
    

