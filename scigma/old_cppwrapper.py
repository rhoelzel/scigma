from ctypes import *
from . import lib
from . import objects
from .blob import Blob
from .dat import Wave
from .num import N_EIGEN_TYPES, N_FLOQUET_TYPES, MODE, MAP,ODE,STROBE,POINCARE

lib.scigma_num_plot.restype=c_void_p
lib.scigma_num_plot.argtypes=[c_char_p,c_int,c_int,c_int,c_int,c_int,c_bool,c_bool]

def plot(nSteps, objlist, showall, noThread,instance):
    eqsys=instance.equationSystem
    eqsysID=eqsys.objectID
    log=instance.log.objectID
    blob=Blob(instance.options['Numerical'])
    blob.set("perval",0.0)
    mode=MODE[instance.options['Numerical']['mode'].label]

    if mode==MAP and nSteps<0:
        if not instance.inverseConsistent:
            raise exception("inverse map and regular map are inconsistent: cannot perform backwards stepping")
        nSteps=-nSteps
        eqsys=instance.inverseEquationSystem
        eqsysID=eqsys.objectID
    if mode==STROBE:
        period=float(eqsys.parse(instance.options['Numerical']['period']))
        if period<0:
            instance.console.write_error("period = "+str(period)+" is negative; using absolute value instead") 
            period = -period
        blob.set("perval", period)
    if mode==POINCARE:
        secvar=instance.options['Numerical']['secvar'].strip()
        secidx=eqsys.variable_names().index(secvar)
        blob.set("secidx", secidx)

    for obj in objlist: 
        objects.move_to(obj,instance)    
        # omit the first point for Poincare maps, because it is usually 
        # not in the Poincare plane
        if mode==POINCARE:
            obj['__varwave__'].destroy()
            nPoints=nSteps if nSteps>0 else -nSteps
            nPoints=nPoints*nperiod if showall else nPoints
            obj['__varwave__']=Wave(None,1+eqsys.n_variables()+eqsys.n_functions(),nPoints)
        identifier=create_string_buffer(bytes(obj['__id__'].encode("ascii")))
        obj['__thread__']=lib.scigma_num_plot(identifier,eqsysID,log,obj['__varwave__'].objectID,nSteps,blob.objectID, showall,noThread)

lib.scigma_num_guess.restype=c_void_p
lib.scigma_num_guess.argtypes=[c_char_p,c_int,c_int,c_int,c_int,c_int,
                              c_double,c_int,c_bool,c_double,c_double,c_int,
                              c_int,c_double,c_double,c_bool,c_bool,
                              c_double,c_double,c_int]

def guess(obj,showall,instance):
    identifier=create_string_buffer(bytes(obj['__id__'].encode("ascii")))
    eqsys=instance.equationSystem.objectID
    log=instance.log.objectID
    varwave=obj['__varwave__'].objectID
    mode=MODE[instance.options['Numerical']['mode'].label]
    period=0
    nperiod=instance.options['Numerical']['nperiod']
    dt=instance.options['Numerical']['dt']
    maxtime=instance.options['Numerical']['maxtime']
    secvar=0
    secdir= 1 if instance.options['Numerical']['secdir'].label=='+' else -1
    secval=instance.options['Numerical']['secval']
    tol=instance.options['Numerical']['Newton']['tol']
    jac = True if instance.options['Numerical']['odessa']['Jacobian'].label=='symbolic' else False
    stiff = True if instance.options['Numerical']['odessa']['type']=='stiff' else False
    atol=instance.options['Numerical']['odessa']['atol']
    rtol=instance.options['Numerical']['odessa']['rtol']
    mxiter=instance.options['Numerical']['odessa']['mxiter']
    
    nVar=obj['__nVar__']
    mode=MODE[instance.options['Numerical']['mode'].label]
    if mode==ODE:
        obj['__evwave__']=Wave(columns=nVar*(nVar+2)+N_EIGEN_TYPES,lines=1)
    else:
        obj['__evwave__']=Wave(columns=nVar*(nVar+2)+N_FLOQUET_TYPES,lines=1)
    evwave=obj['__evwave__'].objectID   
    
    if mode==STROBE:
        # period is assigned here because it might raise an exception which does not matter in other modes
        period=float(instance.equationSystem.parse(instance.options['Numerical']['period']))
        if period<0:
            instance.console.write_error("period = "+str(period)+" is negative; using absolute value instead") 
            period = -period
    if mode==POINCARE:
        # secvar is assigned here because it might raise an exception which does not matter in other modes
        secvar=instance.options['Numerical']['secvar'].strip()
        secvar=instance.equationSystem.variable_names().index(secvar)
    
    obj['__thread__']=lib.scigma_num_guess(identifier,eqsys,log,varwave,evwave,mode,period,nperiod,showall,
                                           dt,maxtime,secvar,secdir,secval,tol,jac,stiff,atol,rtol,mxiter)


lib.scigma_num_map_manifold.restype=c_void_p
lib.scigma_num_map_manifold.argtypes=[c_char_p,c_int,c_int,c_int,c_double,c_double,
                                      POINTER(c_int),c_double,c_double, c_int,c_int,
                                      c_double,c_int,c_bool,c_double,c_double,c_int,c_int,
                                      c_double,c_double,c_bool,c_bool,c_double,c_double,c_int,c_bool]

def map_manifold(nSteps,eival,obj,showall,noThread,instance):
    identifier=create_string_buffer(bytes(obj['__id__'].encode("ascii")))
    eqsys=instance.equationSystem
    eqsysID=eqsys.objectID
    log=instance.log.objectID
    varwave=obj['__varwave__'].objectID
    mode=MODE[instance.options['Numerical']['mode'].label]
    period=0
    nperiod=instance.options['Numerical']['nperiod']
    dt=instance.options['Numerical']['dt']
    maxtime=instance.options['Numerical']['maxtime']
    secvar=0
    secdir= 1 if instance.options['Numerical']['secdir'].label=='+' else -1
    secval=instance.options['Numerical']['secval']
    tol=instance.options['Numerical']['Newton']['tol']
    jac = True if instance.options['Numerical']['odessa']['Jacobian'].label=='symbolic' else False
    stiff = True if instance.options['Numerical']['odessa']['type']=='stiff' else False
    atol=instance.options['Numerical']['odessa']['atol']
    rtol=instance.options['Numerical']['odessa']['rtol']
    mxiter=instance.options['Numerical']['odessa']['mxiter']
    eps=instance.options['Numerical']['manifolds']['eps']
    ds=instance.options['Numerical']['manifolds']['ds']
    alpha=instance.options['Numerical']['manifolds']['alpha']
    
    segmentID=c_int(-1)
    
    if mode==MAP and nSteps<0:
        if not instance.inverseConsistent:
            raise exception("inverse map and regular map are inconsistent: cannot perform backwards stepping")
        nSteps=-nSteps
        eqsys=instance.inverseEquationSystem
        eqsysID=eqsys.objectID
    if mode==STROBE:
        # period is assigned here because it might raise an exception which does not matter in other modes
        period=float(eqsys.parse(instance.options['Numerical']['period']))
        if period<0:
            instance.console.write_error("period = "+str(period)+" is negative; using absolute value instead") 
            period = -period
    if mode==POINCARE:
        # secvar is assigned here because it might raise an exception which does not matter in other modes
        secvar=instance.options['Numerical']['secvar'].strip()
        secvar=eqsys.variable_names().index(secvar)
    
    obj['__thread__']=lib.scigma_num_map_manifold(identifier,eqsysID,log,varwave,eival,eps,byref(segmentID),ds,alpha,
                                                  mode,nSteps,period,nperiod,showall,dt,maxtime,secvar,secdir,secval,
                                                  tol,jac,stiff,atol,rtol,mxiter,noThread)

