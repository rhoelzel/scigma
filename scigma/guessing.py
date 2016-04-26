from ctypes import *
from . import gui
from . import dat
from . import common
from . import lib
from . import windowlist
from . import graphs
from . import picking
from . import iteration

commands={}

N_EIGEN_TYPES=6

REAL_STABLE=0
REAL_NEUTRAL=1
REAL_UNSTABLE=2
COMPLEX_STABLE=3
COMPLEX_NEUTRAL=4
COMPLEX_UNSTABLE=5

EIGEN_INDEX={'RS':REAL_STABLE,'RN':REAL_NEUTRAL,'RU':REAL_UNSTABLE,
             'CS':COMPLEX_STABLE,'CN':COMPLEX_NEUTRAL,'CU':COMPLEX_UNSTABLE}
EIGEN_TYPE=['RS','RN','RU','CS','CN','CU']

N_FLOQUET_TYPES=9

REAL_POSITIVE_STABLE=0
REAL_POSITIVE_NEUTRAL=1
REAL_POSITIVE_UNSTABLE=2
COMPLEX_STABLE=3
COMPLEX_NEUTRAL=4
COMPLEX_UNSTABLE=5
REAL_NEGATIVE_STABLE=6
REAL_NEGATIVE_NEUTRAL=7
REAL_NEGATIVE_UNSTABLE=8

FLOQUET_INDEX={'RPS':REAL_POSITIVE_STABLE,'RPN':REAL_POSITIVE_NEUTRAL,'RPU':REAL_POSITIVE_UNSTABLE,
               'CS':COMPLEX_STABLE,'CN':COMPLEX_NEUTRAL,'CU':COMPLEX_UNSTABLE,
               'RNS':REAL_NEGATIVE_STABLE,'RNN':REAL_NEGATIVE_NEUTRAL,'RNU':REAL_NEGATIVE_UNSTABLE}
FLOQUET_TYPE=['RPS','RPN','RPU','CS','CN','CU','RNS','RNN','RNU']

def guess(path=None,win=None,showall=False):
    """ guess [path]

    Starting at the current position in phase space and parameter 
    space, search for a steady state of the current map/ODE with
    a Newton iteration. If successful, the newly found fixed point
    is displayed with a marker
    The point is stored using the identifier path, or with a generic 
    name of the form 'fpN' (for odes) or 'ppN' (for maps), if no 
    path is given.
    """

    win = windowlist.fetch(win)
    
    nVars=win.eqsys.n_vars()
    
    if nVars == 0:
        raise Exception ("error: no variables defined")

    mode=win.equationPanel.get('mode')
    blob=iteration.blob(win)
    
    if not path:
        rootpath=""
        if mode =='ode':
            prefix='fp'
        else:
            prefix='pp'
    else:
        rootpath=path.rpartition('.')[0]+path.rpartition('.')[1]
        prefix=path.rpartition('.')[2]

    varying=['t']+win.eqsys.var_names()+win.eqsys.func_names()
    const=win.eqsys.par_names()+win.eqsys.const_names()

    nParts = win.cursor['__nparts__']
    
    for varVals, constVals in picking.points(win, varying, const):
        if nParts>1 or not path:
            adjpath=rootpath+graphs.gen_ID(prefix,win)
        else:
            adjpath=rootpath+prefix

        guess_single(adjpath,blob,varying,const,varVals,constVals,win,showall) 
        
"""
        new(win,path,nPoints=1,nParts=1,meshVals=None,varying=None,const=None,varVals=None,constVals=None,fdestroy=None):
        
    nperiod=win.equationPanel.get('nperiod')   
    
    n = nperiod if (showall and mode!='ode') else 1 
    
    # get a dictionary to store data and graph handles
    if path:
        objlist=objects.newlist(path,n,instance)
    else:
        objlist=objects.newlist(objects.new_identifier(defpath,instance),n,instance)
    
    marker = instance.options['Style']['marker']['style']
    marker = marker.definition['none']
    markerSize = 1.0
    point = instance.options['Style']['point']['style']
    point = point.definition[point.label]
    pointSize = instance.options['Style']['marker']['size'].value
    color=instance.options['Style']['color']
    delay=0.0
    nperiod=instance.options['Numerical']['nperiod']   
    
    for obj in objlist:
        objects.move_to(obj,instance)
        obj['__nVar__']=nVar
        cppwrapper.guess(obj,showall,instance)
        obj['__type__']='pt'
        obj['__graph__']=Curve(instance.glWindow,obj['__id__'],n,
                               obj['__varwave__'],obj['__constwave__'],
                               marker,point,markerSize,pointSize,color,0.0,
                               lambda identifier:instance.select_callback(identifier))
        objects.show(obj,instance)
    
    instance.pendingTasks=instance.pendingTasks+len(objlist)
    return objlist
    """

commands['g']=commands['gu']=commands['gue']=commands['gues']=commands['guess']=guess

def guessall(path=None,win=None):
    return guess(path, win, True)

commands['g*']=commands['gu*']=commands['gue*']=commands['gues*']=commands['guess*']=guessall


def rtime(identifier=None, win=None):
    """ rtime [name]

    prints and returns the return time of a set of orbits or
    fixed points of a stroboscopic/Poincare map.
    The orbits specified by objlist; if objlist is not given,
    or the currently selected list of objects is used. 
    Gives an error message if an entry of the specified object
    list does not have a return time
    """
    win = windowlist.fetch(win)
    g = graphs.get(identifier,win) if identifier else picking.selection(win)

    try:
        win.glWindow.stall()
        rt=g['__rtime__']
    except:
        raise Exception('selected object has no return time')
    finally:
        win.glWindow.flush()
    win.console.write_data(str(float("{0:.12f}".format(rt)))+'\n')
    return rt

commands['rt']=commands['rti']=commands['rtim']=commands['rtime']=rtime

def evals(identifier=None, win=None):
    """ evals [name]

    prints and returns the eigenvalues/floquet multipliers
    of the object specified by identifier, or of the current 
    selection , if identifier is omitted.
    Gives an error message if the specified object does not 
    have eigenvalue information
    """
    win = windowlist.fetch(win)
    g = graphs.get(identifier,win) if identifier else picking.selection(win)

    try:
        win.glWindow.stall()
        try:
            evreal = g['__evreal__']
            evimag = g['__evimag__']
        except:
            raise Exception('selected object has no eigenvalue data')
        for i in range(len(evreal)):
            win.console.write(str(i+1).rjust(4)+":  ")
            if evimag[i]<0:
                sign = " - i*" 
            elif evimag[i]>0:
                sign = " + i*"
            else:
                sign = None
            win.console.write_data(str(float("{0:.12f}".format(evreal[i]))))
            if sign==" - i*":
                win.console.write_data(sign+str(float("{0:.12f}".format(-evimag[i]))))
            elif sign==" + i*":
                win.console.write_data(sign+str(float("{0:.12f}".format(evimag[i]))))
            if g['__mode__']!= 'ode':
                modulus = math.sqrt(evreal[i]*evreal[i]+evimag[i]*evimag[i])
                win.console.write("   (modulus: ")
                win.console.write_data(str(float("{0:.12f}".format(modulus))))
                win.console.write(")")
            win.console.write(' \n')
    finally:
        win.glWindow.flush()
    return evreal, evimag

commands['ev']=commands['eva']=commands['eval']=commands['evals']=evals 

def evecs(identifier=None, win=None):
    """ evecs [name]

    prints and returns the eigenvectors
    of the object specified by identifier, or of the current 
    selection , if identifier is omitted.
    Gives an error message if the specified object does not 
    have eigenvector information
    """
    win = windowlist.fetch(win)
    g = graphs.get(identifier,win) if identifier else picking.selection(win)

    try:
        win.glWindow.stall()
        try:
            evecs = g['__evecs__']
            evimag = g['__evimag__']
            nVar = len(evecs)
        except:
            raise Exception('selected object has no eigenvector data')
        impart=False
        for i in range(nVar):
            win.console.write(str(i+1).rjust(4)+":  ")
            line ='('
            if(evimag[i]==0):
                for j in range(nVar):
                    line+=str(float("{0:.12f}".format(evecs[i][j])))+","
            else:
                if not impart:
                    impart=True
                    for j in range(nVar):
                        if evecs[i+1][j]<0:
                            sign = " - i*" 
                        elif evecs[i+1][j]>0:
                            sign = " + i*"
                        else :
                            sign = None
                        line+=str(float("{0:.12f}".format(evecs[i][j])))
                        if sign==" - i*":
                            line+=(sign+str(float("{0:.12f}".format(-evecs[i+1][j]))))
                        elif sign==" + i*":
                            line+=(sign+str(float("{0:.12f}".format(evecs[i+1][j]))))
                        line+=' , '
                else:
                    impart=False
                    for j in range(nVar):
                        if evecs[i][j]<0:
                            sign = " + i*" 
                        elif evecs[i][j]>0:
                            sign = " - i*"
                        else :
                            sign = None
                        line+=str(float("{0:.12f}".format(evecs[i-1][j])))
                        if sign==" - i*":
                            line+=(sign+str(float("{0:.12f}".format(-evecs[i][j]))))
                        elif sign==" + i*":
                            line+=(sign+str(float("{0:.12f}".format(evecs[i][j]))))
                        line+=' , '
            win.console.write_data(line.strip(' , ')+')\n')
    finally:
        win.glWindow.flush()
    return evecs

commands['eve']=commands['evec']=commands['evecs']=evecs 


lib.scigma_num_guess.restype=c_void_p
lib.scigma_num_guess.argtypes=[c_char_p,c_int,c_int,c_int,c_int,c_int,c_bool,c_bool]

def guess_single(path,blob,varying,const,varVals,constVals,win,showall):
    mode = win.equationPanel.get("mode")
    nperiod = win.equationPanel.get("nperiod")
    nPoints = nperiod if (showall and mode!='ode') else 1

    graph=graphs.new(win,path,nPoints,1,None,varying,const,varVals,constVals,finit=init,fcleanup=cleanup)
    graph['__mode__']=mode
    
    nVars=win.eqsys.n_vars()
    eqsysID=win.eqsys.objectID
    logID=win.log.objectID
    blobID=blob.objectID

    identifier=create_string_buffer(bytes(path.encode("ascii")))
    varWaveID=graph['__varwave__'].objectID

    if mode=='ode':
        graph['__evwave__']=dat.Wave(columns=nVars*(nVars+2)+N_EIGEN_TYPES,rows=1)
    else:
        graph['__evwave__']=dat.Wave(columns=nVars*(nVars+2)+N_FLOQUET_TYPES,lines=1)
    evWaveID=graph['__evwave__'].objectID

    graph['__thread__']=lib.scigma_num_guess(identifier,eqsysID,logID,varWaveID,evWaveID,blobID,showall,False)

def init(identifier, win):
    g=graphs.get(identifier, win)
    
    # gather and display some additional information for the fixed/periodic point:   
    nVar=len(g['__varying__'])-1
    evreal=g['__evwave__'][:nVar]
    evimag=g['__evwave__'][nVar:2*nVar]
    evecs=[g['__evwave__'][2*nVar+j*nVar:2*nVar+(j+1)*nVar] for j in range(nVar)]
        
    win.console.write("found steady state:\n")
    vardata=g['__varwave__'].data()
    rows=g['__varwave__'].rows()
    columns=g['__varwave__'].columns()
    names=g['__varying__']
    for i in range(nVar):
        win.console.write(names[i+1]+' = ')
        win.console.write_data(str(float("{0:.10f}".format(vardata[i+1+columns*(rows-1)])))+'\n')
        
    evtypes=[int(t) for t in g['__evwave__'][nVar*(nVar+2):]]
    win.console.write("stability: ")
    info=''
    if g['__mode__']=='ode':
        g['__stable__']=True if evreal[-1]<=0 else False
        for i in range(N_EIGEN_TYPES):
            if evtypes[i]>0:
                info+=(str(evtypes[i])+EIGEN_TYPE[i]+' / ')
    else:
        g['__stable__']=True if evreal[-1]*evreal[-1]+evimag[-1]*evimag[-1]<=1 else False
        for i in range(N_FLOQUET_TYPES):
            if evtypes[i]>0:
                info+=(str(evtypes[i])+FLOQUET_TYPE[i]+' / ')
    win.console.write_data(info.strip(' / ')+'\n')
        
    g['__evreal__']=evreal
    g['__evimag__']=evimag
    g['__evecs__']=evecs
    g['__evtypes__']=evtypes
        
    # if we plotted a stroboscopic map/ Poincare map or guessed steady state of one of those, store the return time
    if g['__mode__']=='strobe' or g['__mode__']=='Poincare':
        varwave=g['__varwave__']
        if varwave.rows()==1:
            g['__rtime__']=varwave[0]
        else:
            g['__rtime__']=varwave[-(nVar+1)]-varwave[-2*(nVar+1)]

            
    # create the visible object        
    if g['__mode__']=='ode':
        pointStyle=gui.RDOT if g['__stable__'] else gui.RING
    else:
        pointStyle=gui.QDOT if g['__stable__'] else gui.QUAD

    pointSize=win.options['Style']['marker']['size'].value
    color=win.options['Style']['color']
    delay=0.0
    
    g['__cgraph__']=gui.Curve(win.glWindow,identifier,g['__npoints__'],g['__nparts__'],
                              g['__mesh__'],g['__varwave__'],g['__constwave__'],
                              gui.NONE,pointStyle,1.0,pointSize,color,0.0,
                              lambda identifier:picking.select(identifier,win))

    graphs.show(g,win)
    picking.select(identifier,win)

def cleanup(identifier,win):
    g=graphs.get(identifier, win)
    g['__evwave__'].destroy()
    
def plug(win=None):
    win = windowlist.fetch(win)
    # make sure that we do not load twice into the same window
    if not win.register_plugin('guessing', lambda:unplug(win), commands):
        return
    
    # fill option panels
    win.glWindow.stall()
    panel=win.acquire_option_panel('Algorithms')
    panel.add('Newton.tol',1e-9)
    panel.define('Newton.tol',"min=0.0")
    enum = common.Enum({'numeric':0,'symbolic':1},'symbolic')
    panel.add('Newton.Jacobian',enum,True,"readonly=true")
    win.glWindow.flush()
    
def unplug(win=None):
    win = windowlist.fetch(win)
    # make sure that we do not unload twice from the same window
    if not win.unregister_plugin('guessing'):
        return
    
    # remove options from panels
    win.glWindow.stall()
    panel=win.acquire_option_panel('Algorithms')
    panel.remove('Newton.tol')
    panel.remove('Newton.Jacobian')
    win.release_option_panel('Algorithms')
    win.glWindow.flush()
