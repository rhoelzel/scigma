from sys import version_info
try:
    if version_info.major == 2:
        # We are using Python 2.x
        import tkFileDialog as tkfile
    elif version_info.major == 3:
        # We are using Python 3.x
        import tkinter.filedialog as tkfile
except:
    print("tkinter not found / not using tk")   
import math
from ctypes import *
from . import lib
from . import default
from .common import Float
from . import options
from . import objects
from . import cppwrapper
from .dat import Wave
from .gui import COORDINATE_NAME, COORDINATE_FLAG, COORDINATE_INDEX, N_COORDINATES, C_COORDINATE, VIEW_TYPE
from .gui import graph, Curve, Navigator, Picker, QDOT, QUAD
from .gui import application
from .num import EquationSystem

""" Available commands:

This file contains all available commands with short descriptions.
Also, for each command a couple of abbreviations is stored in the
'alias' dictionary (so that, for example, the command 'guess' can 
be invoked by typing either 'g', 'gu', 'gue', 'gues', or 'guess').
In the descriptions, mandatory arguments are denoted like <this>,
optional arguments are denoted like [this].
 
"""
alias={}



def delete(objstring, instance=None):
    """delete <object>

    Deletes the object with the specified identifier.
    """
    if not instance:
        instance=default.instance
    try:
        instance.glWindow.stall()
        objlist=objects.get(objstring,instance)
        for obj in objlist:
            objects.delete(obj, instance)
        id = objlist[0]['__id__'].partition('[')[0]
        del instance.objects[id]
    finally:
        instance.glWindow.flush()

alias['del']=alias['dele']=alias['delet']=alias['delete']=delete

def clear(instance=None):
    """ clear

    Deletes all graphical objects.
    """
    if not instance:
        instance=default.instance
    instance.glWindow.stall()
    keys=[key for key in instance.objects]
    for key in keys:
        delete(key,instance)
    instance.glWindow.flush()
    instance.identifiers={}

alias['cl']=alias['cle']=alias['clea']=alias['clear']=clear

def mark(instance=None):
    "marks a periodic point with period > 1"
    if not instance:
        instance=default.instance
        
    
    nperiod=instance.options['Numerical']['nperiod']
    mode=instance.options['Numerical']['mode'].label
    
    origlist=objects.get(None,instance)
    objlist=objects.newlist(objects.new_identifier("ppp",instance),nperiod,instance)
    if origlist[0]['__type__']!='pt' or origlist[0]['__mode__'] == 'ode':
        raise Exception("Can only mark periodic points")
    
    marker = instance.options['Drawing']['marker']['style']
    marker = marker.definition['none']
    markerSize = 1.0
    pointSize = instance.options['Drawing']['marker']['size'].value
    color=instance.options['Drawing']['color']
    delay=0.0
    
    instance.options['Numerical']['nperiod']=1
    instance.options['Numerical']['mode'].label=objlist[0]['__mode__']
    
    # start plotting
    cppwrapper.plot(nperiod-1,objlist,instance)
    # now create the curve
    i = 0
    for obj in objlist:   
        obj['__type__']='pt'
        point = QDOT if origlist[i]['__stable__'] else QUAD
        obj['__graph__']=Curve(instance.glWindow,obj['__id__'],
                               nperiod,obj['__varwave__'],obj['__constwave__'],
                               marker,point,markerSize,pointSize,color,delay,
                               lambda identifier:instance.select_callback(identifier))
        objects.show(obj,instance)
        i=i+1
    
    instance.options['Numerical']['nperiod']=nperiod
    instance.options['Numerical']['mode'].label=mode
    
    instance.pendingTasks=instance.pendingTasks+len(objlist)
    return objlist

alias['ma']=alias['mar']=alias['mark']=mark

def plot(nSteps=1,name=None,instance=None,showall=False,noThread=False):
    """ plot [n] [name]
    
    Performs and plots n iterations of the current map or n time steps
    of the ODE integration, starting at the current position in phase 
    space and parameter space. Makes a single step if n is not given.
    The resulting trajectory is displayed in the current line style 
    (see 'linestyle' and 'linewidth' and 'color' commands). The data
    is stored in the dictionary obj, which is also returned. name is
    used as identifier, or a generic name of the form 'tr<N>' if
    name is not given.     
    """
    if not instance:
        instance=default.instance
    try:
        nSteps = int(nSteps)
    except ValueError:
        raise Exception("error: could not read N (usage: plot [n] [name])")
    
    if instance.equationSystem.n_variables() == 0:
        raise Exception ("error: no variables defined")
    
    n = nSteps if nSteps>0 else -nSteps
    mode=instance.options['Numerical']['mode'].label
    nperiod=instance.options['Numerical']['nperiod']   
    
    n = n*nperiod if (showall and mode!='ode') else n 
    
    if name:
        objlist=objects.newlist(name,n+1,instance)
    else:
        objlist=objects.newlist(objects.new_identifier("tr",instance),n+1,instance)
        
    marker = instance.options['Drawing']['marker']['style']
    marker = marker.definition[marker.label]
    point = instance.options['Drawing']['point']['style']
    point = point.definition[point.label]
    markerSize = instance.options['Drawing']['marker']['size'].value
    pointSize = instance.options['Drawing']['point']['size'].value
    color=instance.options['Drawing']['color']
    delay=instance.options['Drawing']['delay'].value
    
    # start plotting
    cppwrapper.plot(nSteps,objlist,showall,noThread,instance)
    # now create the curve
    for obj in objlist:   
        obj['__type__']='tr'
        obj['__graph__']=Curve(instance.glWindow,obj['__id__'],
                               n+1,obj['__varwave__'],obj['__constwave__'],
                               marker,point,markerSize,pointSize,color,delay,
                               lambda identifier:instance.select_callback(identifier))
        objects.show(obj,instance)
    
    instance.pendingTasks=instance.pendingTasks+len(objlist)
    return objlist

alias['p']=alias['pl']=alias['plo']=alias['plot']=plot

def plotall(nSteps=1,name=None,instance=None,noThread=False):
    return plot(nSteps,name,instance,True,noThread)

alias['p*']=alias['pl*']=alias['plo*']=alias['plot*']=plotall


def manifold(stable,n=1,originlist=None,name=None,instance=None,showall=False,noThread=False):
    if not instance:
        instance=default.instance
    try:
        n = int(n)
    except ValueError:
        raise Exception("error: could not read N (usage: mu/ms [n] [name])")
    if n<0:
        n=-n
    
    # the object from which to generate the manifold
    origobjlist=objects.get(originlist,instance)
    
    # check, whether the structure of the equation system is still the same
    if origobjlist[0]['__timestamp__']!=instance.timeStamp:
        id=origobjlist[0]['__id__'].partition('[')[0]
        raise Exception("structure of equation system has changed - cannot compute invariant manifold of object "+ id)
    
    # get a dictionary to store data and graph handles
    if name:
        objlist=objects.newlist(name,1,instance)
    else:
        objlist=objects.newlist(objects.new_identifier("mf",instance),1,instance)
    
    eps=instance.options['Numerical']['manifolds']['eps']
    marker = instance.options['Drawing']['marker']['style']
    marker = marker.definition[marker.label]
    point = instance.options['Drawing']['point']['style']
    point = point.definition[point.label]
    markerSize = instance.options['Drawing']['marker']['size'].value
    pointSize = instance.options['Drawing']['point']['size'].value
    color=instance.options['Drawing']['color']
    delay=instance.options['Drawing']['delay'].value
    evindex=instance.options['Numerical']['manifolds']['evec1']
    
    mode=instance.options['Numerical']['mode'].label
    nperiod=instance.options['Numerical']['nperiod']
    nPoints = ((n+1)*nperiod) if (showall and mode !='ode') else (n+1)
    
    for origobj,obj in zip(origobjlist,objlist):
        # see if we can use the specified origin to create an unstable manifold
        try:
            evreal = origobj['__evreal__']
            evimag = origobj['__evimag__']
            evecs = origobj['__evecs__']
        except:
            raise Exception(origobj['__id__']+': origin has no eigenvalue and/or eigenvector data')
        
        if stable and evimag[evindex-1]!=0:
            raise Exception(origobj['__id__']+': eigenvector number '+ str(len(evreal)-evindex+1) +' has complex eigenvalue (evec1='+str(evindex)+')')
        elif not stable and evimag[-evindex]!=0:
            raise Exception(origobj['__id__']+': eigenvector number '+ str(len(evreal)-evindex+1) +' has complex eigenvalue (evec1='+str(evindex)+')')
        
        if origobj['__mode__']=='ode':
            if stable and evreal[evindex-1]>=0:
                raise Exception(origobj['__id__']+': real part of eigenvalue number ' + str(len(evreal)-evindex+1) +' is not negative (evec1='+str(evindex)+')')
            elif not stable and evreal[-evindex]<=0:
                raise Exception(origobj['__id__']+': real part of eigenvalue number ' + str(len(evreal)-evindex+1) +' is not positive (evec1='+str(evindex)+')')
        else:
            if stable and evreal[evindex-1]*evreal[evindex-1]+evimag[evindex-1]*evimag[evindex-1]>=1:
                raise Exception(origobj['__id__']+': modulus of multiplier number ' + str(len(evreal)-evindex+1) +' is not < 1 (evec1='+str(evindex)+')')
            elif not stable and evreal[-evindex]*evreal[-evindex]+evimag[-evindex]*evimag[-evindex]<=1:
                raise Exception(origobj['__id__']+': modulus of multiplier number ' + str(len(evreal)-evindex+1) +' is not > 1 (evec1='+str(evindex)+')')
        
        # print information about selected epsilon, eigenvalue and eigenvector
        instance.glWindow.stall()
        instance.console.write("eps: ")
        instance.console.write_data(str(instance.options['Numerical']['manifolds']['eps'])+'\n')
        instance.console.write("eigenvalue: ")
        if stable:
            instance.console.write_data(str(evreal[evindex-1])+'\n')
        else:
            instance.console.write_data(str(evreal[-evindex])+'\n')
        instance.console.write("eigenvector: ")
        line ='('
        for j in range(len(evreal)):
            if stable:
                line+=str(float("{0:.12f}".format(evecs[evindex-1][j])))+","
            else:
                line+=str(float("{0:.12f}".format(evecs[-evindex][j])))+","
        line=line.strip(',')+')\n'
        instance.console.write_data(line)
        instance.glWindow.flush()
        
        fp = origobj['__varwave__'].data()
        rows = origobj['__varwave__'].rows()
        columns = origobj['__varwave__'].columns()
        init=[]
        for i in range(columns):
            init.append(fp[i+columns*(rows-1)])
        init.append(fp[columns*(rows-1)])
        if stable:
            for i in range(len(evreal)):
                init.append(fp[i+1+columns*(rows-1)]+eps*evecs[evindex-1][i])
        else:
            for i in range(len(evreal)):
                init.append(fp[i+1+columns*(rows-1)]+eps*evecs[-evindex][i])
        
        # for the next bit, we are cheating, because we do not
        # actually evaluate the values of dependent functions
        # for the initial segment - instead, we copy the values
        # for the fixed point
        for i in range(1+len(evreal),columns):
            init.append(fp[i+columns*(rows-1)])
            
        obj['__varwave__'].destroy()
        obj['__varwave__']=Wave(init,columns,lines=nPoints)


        objects.move_to(obj,instance)
        
    if n>1:
        if origobjlist[0]['__mode__']=='ode':
            nSteps= 1-n if stable else n-1
            cppwrapper.plot(nSteps,objlist,showall,noThread,instance)
        else:
            for obj in objlist:
                eival = 1/evreal[evindex-1] if stable else evreal[-evindex]
                nSteps = 1-n if stable else n-1
                cppwrapper.map_manifold(nSteps,eival,obj,showall,noThread,instance)
        
    # create the curves
    for obj in objlist:
        obj['__type__']='mf' if stable else 'mu'
        obj['__graph__']=Curve(instance.glWindow,obj['__id__'],nPoints,
                               obj['__varwave__'],obj['__constwave__'],
                               marker,point,markerSize,pointSize,color,delay,
                               lambda identifier:instance.select_callback(identifier))
        objects.show(obj,instance)
        
    instance.pendingTasks=instance.pendingTasks+len(objlist)
    return objlist

def munstable(n=1,origin=None,name=None,instance=None,noThread=False):
    """ munstable [n] [origin] [name]
    
    Starting at the fixed point or periodic point with the
    identifier origin, create the 1-dimensional unstable manifold along 
    the eigenvector specified by options['Numerical']['manifolds']['evec1'].
    options['Numerical']['manifolds']['ds'] is used as initial perturbation
    from the fixed point along the specified eigenvector. Plot n time steps
    / arclength
    The point is stored using the identifier name, or with a generic 
    name of the form 'mfN', if no name is given. 
    The behavior is analog for mstable.
    """
    return manifold(False,n,origin,name,instance,False,noThread)
alias['mu']=alias['mun']=alias['muns']=alias['munst']=alias['munsta']=alias['munstab']=alias['munstabl']=alias['munstable']=munstable
def mstable(n=1,origin=None,name=None,instance=None,noThread=False):
    return manifold(True,n,origin,name,instance,False,noThread)
alias['ms']=alias['mst']=alias['msta']=alias['mstab']=alias['mstabl']=alias['mstable']=mstable
def munstableall(n=1,origin=None,name=None,instance=None,noThread=False):
    return manifold(False,n,origin,name,instance,True,noThread)
alias['mu*']=alias['mun*']=alias['muns*']=alias['munst*']=alias['munsta*']=alias['munstab*']=alias['munstabl*']=alias['munstable*']=munstableall
def mstableall(n=1,origin=None,name=None,instance=None,noThread=False):
    return manifold(True,n,origin,name,instance,True,noThread)
alias['ms*']=alias['mst*']=alias['msta*']=alias['mstab*']=alias['mstabl*']=alias['mstable*']=mstableall


def select(objstring, instance=None):
    """select <object>

    Selects the object with the specified identifier.
    """
    if not instance:
        instance=default.instance
    objlist=objects.get(objstring,instance)
    objects.select(objlist, instance)

alias['sel']=alias['sele']=alias['selec']=alias['select']=select



def equations(filename=None,instance=None):
    """ equations [filename]

    Loads a python file with equations
    """
    if not instance:
        instance=default.instance
    if not filename:
        filename=tkfile.askopenfilename()
    if filename=='internal':
        instance.equationSystem=instance.equationSystemBackup
        instance.equationPanel.define("","visible=true")
        instance.source='internal'
    elif filename:
        instance.equationSystemBackup=instance.equationSystem
        instance.equationSystem=EquationSystem(filename)
        instance.equationPanel.define("","visible=false")
        instance.source=filename
    instance.glWindow.set_title("SCIGMA - script: "+instance.script+" - equations: "+instance.source)
    instance.rebuild_panels()

alias['eq']=alias['equ']=alias['equa']=alias['equat']=alias['equati']=alias['equatio']=alias['equation']=alias['equations']=equations


def reset(instance=None):
    """ reset
    Deletes all graphical objects and all equations, and
    resets the viewing volumme
    """
    if not instance:
        instance=default.instance
    clear(instance)
    xexpr('x',instance)
    yexpr('y',instance)
    zexpr('',instance)
    axes('xy', instance)
    x_range(-1,1,instance)
    y_range(-1,1,instance)
    z_range(-1,1,instance)
    c_range(0,1,instance)
    instance.glWindow.reset_rotation()
    instance.script='none'
    instance.source='internal'
    instance.equationSystemBackup=EquationSystem()
    equations('internal',instance)

alias['res']=alias['rese']=alias['reset']=reset




def set(function, value, instance=None):
    if not instance:
        instance = default.instance
    result=instance.equationSystem.parse(function+"="+str(value))
    if(result[0:6]== "error:"):
        raise Exception(result[6:])
    instance.timeStamp=instance.equationSystem.timestamp()
    objects.move_cursor(None,instance)

def wait(seconds):
    application.loop(seconds)

