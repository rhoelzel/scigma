import math
from . import gui
from . import common
from . import library
from . import options
from . import graphs
from . import windowlist
from . import equations

commands={}
timestamp={}

def circle(d,n=100,win=None):
    """ circle [name]                                                                                                   
                                                                                                                        
    creates a circle of initial conditions;                                                                             
    d is the diameter measured in units of                                                                              
    the coordinate system                                                                                               
    """
    win = windowlist.fetch(win)

    d=float(d)
    n=int(n)

    if n<2:
        raise Exception('circle must have at least two points')
    
    xexp=win.options['View']['x']
    yexp=win.options['View']['y']
    if not (xexp in win.eqsys.var_names()
            or xexp in win.eqsys.par_names()):
        raise Exception('cannot set initial conditions as long as x-axis shows '+xexp)
    if not (yexp in win.eqsys.var_names()
            or yexp in win.eqsys.par_names()):
        raise Exception('cannot set initial conditions as long as y-axis shows '+yexp)
    xval=float(win.eqsys.parse('$'+xexp))
    yval=float(win.eqsys.parse('$'+yexp))

    min,max=win.glWindow.range()
    dx=d*(max[0]-min[0])*0.5
    dy=d*(max[1]-min[1])*0.5
    dphi=2*math.pi/n
    phi=0
    varying=[]
    varVals=[]
    win.eqsys.parse(xexp+"="+str(xval+math.cos(phi)*dx))
    const, constVals = equations.point(win)
    for i in range(0,n-1):
        phi=phi+dphi
        win.eqsys.stall()
        win.eqsys.parse(xexp+"="+str(xval+math.cos(phi)*dx))
        win.eqsys.parse(yexp+"="+str(yval+math.sin(phi)*dy))
        win.eqsys.flush()
        names, vals = equations.point(win)
        update_varying_const(i+1,varying,varVals,const,constVals,names,vals)

    graphs.destroy(win.cursor,win)

    win.cursor = graphs.new(win,None,1,n,None,varying,const,varVals,constVals,None)
    win.cursor['__cgraph__']=gui.Curve(win.glWindow,'__cursor__',1,win.cursor['__nparts__'],
                                       win.cursor['__mesh__'],win.cursor['__varwave__'],win.cursor['__constwave__'],
                                       gui.NONE,gui.RCROSS,0.0,50 if library.largeFontsFlag else 25,[1.0,1.0,1.0,1.0],0.0)
    graphs.show(win.cursor,win)
        
commands['cir']=commands['circ']=commands['circl']=commands['circle']=circle

def fill(n=10, win=None):
    win = windowlist.fetch(win)

    nFill=int(n)

    if nFill<2:
        raise Exception('must create at least two new segments')

    nParts=win.cursor['__nparts__']
    if nParts<2:
        raise Exception('need at least two points to interpolate')


    varying = win.cursor['__varying__']
    varWave=win.cursor['__varwave__']
    varWave.lock()
    varVals=[]
    varCVals=win.cursor['__varwave__'].data()
    nVars=len(varying)
    
    for k in range(nParts-1):
        for j in range(nFill):
            for i in range(nVars):
                varVals.append(varCVals[k*nVars+i]+(varCVals[(k+1)*nVars+i]-varCVals[k*nVars+i])*float(j)/float(nFill))
    for i in range(nVars):
        varVals.append(varCVals[(nParts-1)*nVars+i])
    varWave.unlock()           

    const = win.cursor['__const__']
    constWave=win.cursor['__constwave__']
    constWave.lock()
    constVals=[]
    constCVals=win.cursor['__constwave__'].data()
    for i in range(constWave.rows()*constWave.columns()):
        constVals.append(constCVals[i])
    constWave.unlock()

    nParts=(nParts-1)*nFill+1
    meshVals=range(nParts)

    oldc=win.cursor

    win.cursor = graphs.new(win,None,1,nParts,meshVals,varying,const,varVals,constVals,None)
    win.cursor['__cgraph__']=gui.Curve(win.glWindow,'__cursor__',1,win.cursor['__nparts__'],
                                       win.cursor['__mesh__'],win.cursor['__varwave__'],win.cursor['__constwave__'],
                                       gui.NONE,gui.RCROSS,0.0,50 if library.largeFontsFlag else 25,[1.0,1.0,1.0,1.0],0.0)
    graphs.show(win.cursor,win)
    # this gives OpenGL some time to load the data into the buffer; not strictly necessary, this reduces flickering
    gui.application.loop(0.1)
    graphs.destroy(oldc,win)

    
commands['fil']=commands['fill']=fill    

def add(win=None):
    win = windowlist.fetch(win)

    names, vals = equations.point(win)

    varVals=win.cursor['__varwave__'][:]
    constVals=win.cursor['__constwave__'][:]

    varying=win.cursor['__varying__']
    const=win.cursor['__const__']

    nParts=win.cursor['__nparts__']+1
    meshVals=range(nParts)
    update_varying_const(nParts-1,varying,varVals,const,constVals,names,vals)

    oldc=win.cursor

    win.cursor = graphs.new(win,None,1,nParts,meshVals,varying,const,varVals,constVals,None)
    win.cursor['__cgraph__']=gui.Curve(win.glWindow,'__cursor__',1,win.cursor['__nparts__'],
                                       win.cursor['__mesh__'],win.cursor['__varwave__'],win.cursor['__constwave__'],
                                       gui.NONE,gui.RCROSS,0.0,50 if library.largeFontsFlag else 25,[1.0,1.0,1.0,1.0],0.0)
    graphs.show(win.cursor,win)
    # this gives OpenGL some time to load the data into the buffer; not strictly necessary, this reduces flickering
    gui.application.loop(0.1)
    graphs.destroy(oldc,win)

def pick(win=None):
    win = windowlist.fetch(win)

    oldc=win.cursor

    win.cursor = graphs.new(win)
    win.cursor['__cgraph__']=gui.Curve(win.glWindow,'__cursor__',1,1,
                                       win.cursor['__mesh__'],win.cursor['__varwave__'],win.cursor['__constwave__'],
                                       gui.NONE,gui.RCROSS,0.0,50 if library.largeFontsFlag else 25,[1.0,1.0,1.0,1.0],0.0)
    graphs.show(win.cursor,win)
    graphs.destroy(oldc,win)
    
def select(identifier, win=None):
    win=windowlist.fetch(win)
    win.selection=graphs.get(identifier, win)
    
    """if not identifier:
        if not win.cursor['__cgraph__']:
            win.cursor['__cgraph__']=gui.Curve(win.glWindow,'__cursor__',win.cursor['__npoints__'],win.cursor['__nparts__'],
                                               win.cursor['__mesh__'],win.cursor['__varwave__'],win.cursor['__constwave__'],
                                               gui.NONE,gui.RCROSS,0.0,50 if library.largeFontsFlag else 25,[1.0,1.0,1.0,1.0],0.0)
            graphs.show(win.cursor,win)
        return
        
    g = graphs.get(identifier,win)"""
    
def where(win=None):
    """ where
    
    Prints the current values of variables and
    parameters at the console.
    """
    win=windowlist.fetch(win)

    varNames=win.eqsys.var_names()
    parNames=win.eqsys.par_names()
    try:
        win.glWindow.stall()
        nParts=win.cursor['__nparts__']
        if nParts>1:
            win.console.write("Currently, "+str(nParts)+" are selected.\n")
            win.console.write("Displaying only position of first point:")

        pt = 1
        for tVal,varVals,parVals in points(win,['t'],varNames,parNames):
            if pt == 1:
                win.console.write('t = '+str(float("{0:.10f}".format(tVal[0])))+'\n')
                if len(varNames): win.console.write("variables:\n")
                for i in range(len(varNames)):
                    win.console.write(varNames[i]+" = ")
                    win.console.write_data(str(float("{0:.10f}".format(varVals[i])))+'\n')
                if len(parNames): win.console.write("parameters:\n")
                for i in range(len(parNames)):
                    win.console.write(parNames[i]+" = ")
                    win.console.write_data(str(float("{0:.10f}".format(parVals[i])))+'\n')
        
    finally:
        win.glWindow.flush()

commands['w']=commands['wh']=commands['whe']=commands['wher']=commands['where']=where 

def selection(win):
    win=windowlist.fetch(win)
    return win.selection

def points(win,*allnames):
    varying, const = win.cursor['__varying__'], win.cursor['__const__']
    varVals, constVals = win.cursor['__varwave__'][:], win.cursor['__constwave__'][:]
    
    nVars = len(varying)
    
    for i in range(win.cursor['__nparts__']):
        result = ()
        for names in allnames:
            values = []
            for name in names:
                try:
                    values.append(varVals[varying.index(name)+i*nVars])
                except:
                    try:
                        values.append(constVals[const.index(name)])
                    except:
                        raise Exception(name+": function not found in equation system; cannot determine coordinates")
            result = result + (values,)
        yield result

def on_pick(win,ctrl,*args):
    allow=True
    for i in range(gui.Z_INDEX+1):
        var = win.options['View'][gui.COORDINATE_NAME[i]]
        if (not (var in win.eqsys.var_names() or 
                 var in win.eqsys.par_names()) and
        args[i]):
            allow = False
    if not allow:
        win.console.write_warning("mouse picking is deactivated\n")
    else:
        for i in range(gui.Z_INDEX+1):
            if(args[i]):
                var=win.options['View'][gui.COORDINATE_NAME[i]]
                win.eqsys.parse(var+"="+str(args[i]))
                win.invsys.parse(var+"="+str(args[i]))
                win.console.write("setting "+var+" to ")
                win.console.write_data(str(args[i])+"\n")
        if ctrl:
            add(win)
        else:
            pick(win)

def on_parse(win):
    # if the structure has not changed, replace the last point of the cursor with
    # the current point; if the structure has changed, create a new single cursor
    if not win.eqsys.timestamp() == timestamp[win]:
        timestamp[win]=win.eqsys.timestamp()
        pick(win)
    else:

        names, vals = equations.point(win)

        varWave=win.cursor['__varwave__']
        varVals=varWave[:-varWave.columns()]
        constVals=win.cursor['__constwave__'][:]
            
        varying=win.cursor['__varying__']
        const=win.cursor['__const__']

        nParts=win.cursor['__nparts__']
        meshVals=range(nParts)

        update_varying_const(nParts-1,varying,varVals,const,constVals,names,vals)
        
        oldc=win.cursor
        
        win.cursor = graphs.new(win,None,1,nParts,meshVals,varying,const,varVals,constVals,None)
        win.cursor['__cgraph__']=gui.Curve(win.glWindow,'__cursor__',1,win.cursor['__nparts__'],
                                           win.cursor['__mesh__'],win.cursor['__varwave__'],win.cursor['__constwave__'],
                                           gui.NONE,gui.RCROSS,0.0,50 if library.largeFontsFlag else 25,[1.0,1.0,1.0,1.0],0.0)
        graphs.show(win.cursor,win)
        # this gives OpenGL some time to load the data into the buffer; not strictly necessary, this reduces flickering
        gui.application.loop(0.1)
        graphs.destroy(oldc,win)

def update_varying_const(nParts,varying,varVals,const,constVals,names,vals):
    nVars=len(varying)

    for i in range(len(names)):
        if names[i] in varying:
            varVals.append(vals[i])
        else:
            isConst = False
            for j in range(len(const)):
                if names[i] is const[j]:
                    if vals[i] == constVals[j]:
                        isConst=True
                    break
            if not isConst:
                # delete label and value from constants
                idx=const.index(names[i])
                value=constVals[idx]
                del const[idx]
                del constVals[idx]

                # insert label and both old and new const values into varyings
                varying.append(names[i])
                for j in range(nParts):
                    varVals.insert(j*(nVars+1)+nVars,value)
                varVals.append(vals[i])
                nVars=nVars+1
        
def plug(win):
    if not win.register_plugin('picking', lambda:unplug(win),commands):
        return

    picker=gui.Picker(gui.VIEW_TYPE[win.options['View']['axes'].label])
    picker.set_callback(lambda ctrl,x,y,z: on_pick(win,ctrl,x,y,z))
    win.glWindow.connect_before(picker)

    setattr(win,'picker',picker)

    cursor = graphs.new(win)
    cursor['__cgraph__']=gui.Curve(win.glWindow,'__cursor__',1,1,cursor['__mesh__'],cursor['__varwave__'],cursor['__constwave__'],
                                  gui.NONE,gui.RCROSS,0.0,50 if library.largeFontsFlag else 25,[1.0,1.0,1.0,1.0],0.0)
    setattr(win,'cursor',cursor)
    graphs.show(cursor,win)

    setattr(win,'selection',None)
    
    timestamp[win]=0
    
def unplug(win):
    if not win.unregister_plugin('picking'):
        return

    win.glWindow.remove_drawable(win.picker)
    win.glWindow.disconnect(win.picker)
    win.picker.destroy()
    
    delattr(win,'picker')

    graphs.destroy(win.cursor,win)
    delattr(win,'cursor')

    delattr(win,'selection')
    
    del timestamp[win]
