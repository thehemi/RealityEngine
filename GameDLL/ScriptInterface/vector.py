# This file was created automatically by SWIG.
import vectorc
class Vector2:
    __setmethods__ = {}
    for _s in []: __setmethods__.update(_s.__setmethods__)
    def __setattr__(self,name,value):
        if (name == "this"):
            if isinstance(value,Vector2):
                self.__dict__[name] = value.this
                if hasattr(value,"thisown"): self.__dict__["thisown"] = value.thisown
                del value.thisown
                return
        method = Vector2.__setmethods__.get(name,None)
        if method: return method(self,value)
        self.__dict__[name] = value

    __getmethods__ = {}
    for _s in []: __getmethods__.update(_s.__getmethods__)
    def __getattr__(self,name):
        method = Vector2.__getmethods__.get(name,None)
        if method: return method(self)
        raise AttributeError,name

    __setmethods__["x"] = vectorc.Vector2_x_set
    __getmethods__["x"] = vectorc.Vector2_x_get
    __setmethods__["y"] = vectorc.Vector2_y_set
    __getmethods__["y"] = vectorc.Vector2_y_get
    def __init__(self,*args):
        self.this = apply(vectorc.new_Vector2,args)
        self.thisown = 1
    def Average(*args): return apply(vectorc.Vector2_Average,args)
    def __call__(*args): return apply(vectorc.Vector2___call__,args)
    def __eq__(*args): return apply(vectorc.Vector2___eq__,args)
    def __imul__(*args): return apply(vectorc.Vector2___imul__,args)
    def __del__(self,vectorc=vectorc):
        if getattr(self,'thisown',0):
            vectorc.delete_Vector2(self)
    def __repr__(self):
        return "<C Vector2 instance at %s>" % (self.this,)

class Vector2Ptr(Vector2):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = Vector2
vectorc.Vector2_swigregister(Vector2Ptr)
class Vector:
    __setmethods__ = {}
    for _s in []: __setmethods__.update(_s.__setmethods__)
    def __setattr__(self,name,value):
        if (name == "this"):
            if isinstance(value,Vector):
                self.__dict__[name] = value.this
                if hasattr(value,"thisown"): self.__dict__["thisown"] = value.thisown
                del value.thisown
                return
        method = Vector.__setmethods__.get(name,None)
        if method: return method(self,value)
        self.__dict__[name] = value

    __getmethods__ = {}
    for _s in []: __getmethods__.update(_s.__getmethods__)
    def __getattr__(self,name):
        method = Vector.__getmethods__.get(name,None)
        if method: return method(self)
        raise AttributeError,name

    __setmethods__["x"] = vectorc.Vector_x_set
    __getmethods__["x"] = vectorc.Vector_x_get
    __setmethods__["y"] = vectorc.Vector_y_set
    __getmethods__["y"] = vectorc.Vector_y_get
    __setmethods__["z"] = vectorc.Vector_z_set
    __getmethods__["z"] = vectorc.Vector_z_get
    def __init__(self,*args):
        self.this = apply(vectorc.new_Vector,args)
        self.thisown = 1
    def __xor__(*args): return apply(vectorc.Vector___xor__,args)
    def __or__(*args): return apply(vectorc.Vector___or__,args)
    def __add__(*args): return apply(vectorc.Vector___add__,args)
    def __sub__(*args): return apply(vectorc.Vector___sub__,args)
    def __div__(*args): return apply(vectorc.Vector___div__,args)
    def __eq__(*args): return apply(vectorc.Vector___eq__,args)
    def __ne__(*args): return apply(vectorc.Vector___ne__,args)
    def __neg__(*args): return apply(vectorc.Vector___neg__,args)
    def __iadd__(*args): return apply(vectorc.Vector___iadd__,args)
    def __isub__(*args): return apply(vectorc.Vector___isub__,args)
    def __imul__(*args): return apply(vectorc.Vector___imul__,args)
    def __idiv__(*args): return apply(vectorc.Vector___idiv__,args)
    def Length(*args): return apply(vectorc.Vector_Length,args)
    def IsNearlyZero(*args): return apply(vectorc.Vector_IsNearlyZero,args)
    def IsZero(*args): return apply(vectorc.Vector_IsZero,args)
    def Normalize(*args): return apply(vectorc.Vector_Normalize,args)
    def Set(*args): return apply(vectorc.Vector_Set,args)
    def Add(*args): return apply(vectorc.Vector_Add,args)
    def Sub(*args): return apply(vectorc.Vector_Sub,args)
    def Mul(*args): return apply(vectorc.Vector_Mul,args)
    def Cmp(*args): return apply(vectorc.Vector_Cmp,args)
    def Yaw(*args): return apply(vectorc.Vector_Yaw,args)
    def Pitch(*args): return apply(vectorc.Vector_Pitch,args)
    def Roll(*args): return apply(vectorc.Vector_Roll,args)
    def Zero(*args): return apply(vectorc.Vector_Zero,args)
    def Normalized(*args): return apply(vectorc.Vector_Normalized,args)
    def DotSelf(*args): return apply(vectorc.Vector_DotSelf,args)
    def Dot(*args): return apply(vectorc.Vector_Dot,args)
    def ScalarProjectionOntoVector(*args): return apply(vectorc.Vector_ScalarProjectionOntoVector,args)
    def ProjectionOntoVector(*args): return apply(vectorc.Vector_ProjectionOntoVector,args)
    def Lerp(*args): return apply(vectorc.Vector_Lerp,args)
    def RadAngle(*args): return apply(vectorc.Vector_RadAngle,args)
    def CosAngle(*args): return apply(vectorc.Vector_CosAngle,args)
    def Reflected(*args): return apply(vectorc.Vector_Reflected,args)
    def DistanceToLine(*args): return apply(vectorc.Vector_DistanceToLine,args)
    def Average(*args): return apply(vectorc.Vector_Average,args)
    def HalfWay(*args): return apply(vectorc.Vector_HalfWay,args)
    def __del__(self,vectorc=vectorc):
        if getattr(self,'thisown',0):
            vectorc.delete_Vector(self)
    def __repr__(self):
        return "<C Vector instance at %s>" % (self.this,)

class VectorPtr(Vector):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = Vector
vectorc.Vector_swigregister(VectorPtr)
class Vector4(Vector):
    __setmethods__ = {}
    for _s in [Vector]: __setmethods__.update(_s.__setmethods__)
    def __setattr__(self,name,value):
        if (name == "this"):
            if isinstance(value,Vector4):
                self.__dict__[name] = value.this
                if hasattr(value,"thisown"): self.__dict__["thisown"] = value.thisown
                del value.thisown
                return
        method = Vector4.__setmethods__.get(name,None)
        if method: return method(self,value)
        self.__dict__[name] = value

    __getmethods__ = {}
    for _s in [Vector]: __getmethods__.update(_s.__getmethods__)
    def __getattr__(self,name):
        method = Vector4.__getmethods__.get(name,None)
        if method: return method(self)
        raise AttributeError,name

    __setmethods__["w"] = vectorc.Vector4_w_set
    __getmethods__["w"] = vectorc.Vector4_w_get
    def __init__(self,*args):
        self.this = apply(vectorc.new_Vector4,args)
        self.thisown = 1
    def Lerp(*args): return apply(vectorc.Vector4_Lerp,args)
    def __mul__(*args): return apply(vectorc.Vector4___mul__,args)
    def __imul__(*args): return apply(vectorc.Vector4___imul__,args)
    def __add__(*args): return apply(vectorc.Vector4___add__,args)
    def __iadd__(*args): return apply(vectorc.Vector4___iadd__,args)
    def __sub__(*args): return apply(vectorc.Vector4___sub__,args)
    def __isub__(*args): return apply(vectorc.Vector4___isub__,args)
    def __div__(*args): return apply(vectorc.Vector4___div__,args)
    def __idiv__(*args): return apply(vectorc.Vector4___idiv__,args)
    def Length(*args): return apply(vectorc.Vector4_Length,args)
    def Average(*args): return apply(vectorc.Vector4_Average,args)
    def Normalized(*args): return apply(vectorc.Vector4_Normalized,args)
    def __del__(self,vectorc=vectorc):
        if getattr(self,'thisown',0):
            vectorc.delete_Vector4(self)
    def __repr__(self):
        return "<C Vector4 instance at %s>" % (self.this,)

class Vector4Ptr(Vector4):
    def __init__(self,this):
        self.this = this
        if not hasattr(self,"thisown"): self.thisown = 0
        self.__class__ = Vector4
vectorc.Vector4_swigregister(Vector4Ptr)
PI = vectorc.PI
SMALL_NUMBER = vectorc.SMALL_NUMBER
KINDA_SMALL_NUMBER = vectorc.KINDA_SMALL_NUMBER
BIG_NUMBER = vectorc.BIG_NUMBER
Vector_MakeDirection = vectorc.Vector_MakeDirection

Vector_NormalFromTriangle = vectorc.Vector_NormalFromTriangle

Cross = vectorc.Cross


