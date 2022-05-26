//
// SWIG exception handling for Ruby
//
// $Header: /cvs/projects/SWIG/Lib/ruby/exception.i,v 1.1 2000/07/05 18:58:47 ttn Exp $
//
// Copyright (C) 2000  Network Applied Communication Laboratory, Inc.
// Copyright (C) 2000  Information-technology Promotion Agency, Japan
//
// Masaki Fukushima
//

%{
#define  SWIG_MemoryError    rb_eFatal
#define  SWIG_IOError        rb_eIOError
#define  SWIG_RuntimeError   rb_eRuntimeError
#define  SWIG_IndexError     rb_eIndexError
#define  SWIG_TypeError      rb_eTypeError
#define  SWIG_DivisionByZero rb_eZeroDivError
#define  SWIG_OverflowError  rb_eRuntimeException
#define  SWIG_SyntaxError    rb_eSyntaxError
#define  SWIG_ValueError     rb_eArgError
#define  SWIG_SystemError    rb_eSystemError
#define  SWIG_UnknownError   rb_eRuntimeError

#define SWIG_exception(a,b) rb_raise(a,b)

%}
