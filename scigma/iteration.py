from ctypes import *
from Tkinter import *
from . import gui
from . import common
from . import lib
from . import library
from . import windowlist
from . import graphs
from . import picking
from . import equations

commands={}

def blob(win):
    mode = win.equationPanel.get("mode")
    nperiod = win.equationPanel.get("nperiod")

    blob=common.Blob(win.options['Algorithms'])
    blob.set("nperiod",nperiod)
    blob.set("period",0.0)
    blob.set("mode", equations.MODE[mode])
    
    if mode=='strobe':
        period=float(win.eqsys.parse(win.equationPanel.get('period')))
        if period<0:
            win.console.write_warning("period = "+str(period)+" is negative; using absolute value instead") 
            period = -period
        blob.set("period", period)
    if mode=='Poincare':
        secvar=win.equationPanel.get("secvar").strip()
        secidx=win.eqsys.var_names().index(secvar)
        blob.set("secidx", secidx)
        secval=win.equationPanel.get("secval")
        blob.set("secval", secval)
        secdir=win.equationPanel.get("secdir")
        blob.set("secdir", 1 if secdir is '+' else -1)
        maxtime=win.equationPanel.get("maxtime")
        blob.set("maxtime",maxtime)

    return blob

lib.scigma_num_plot.restype=c_int
lib.scigma_num_plot.argtypes=[c_char_p,c_int,c_int,c_int,c_int,c_int,c_char_p,c_int,c_bool,c_bool]

def plot(nSteps=1,path=None,win=None,showall=False,noThread=False):
    win = windowlist.fetch(win)

    try:
        nSteps = int(nSteps)
    except ValueError:
        raise Exception("error: could not read nSteps (usage: plot [nSteps] [name])")
    
    nVars=win.eqsys.n_vars()
    
    if nVars == 0:
        raise Exception ("error: no variables defined")

    blb=blob(win)
    if not path:
        path=graphs.gen_ID("tr",win)

    mode = win.equationPanel.get("mode")
    nperiod = win.equationPanel.get("nperiod")
    nPoints = nperiod*abs(nSteps) if (showall and mode!='ode') else abs(nSteps)

    
    
    varying, const, varVals, constVals = collect_varying_const(win,
                                                               win.cursor['nparts'],
                                                               win.cursor['varying'],
                                                               win.cursor['const'][:],
                                                               win.cursor['varwave'][:],
                                                               win.cursor['constwave'][:])

    varPars='|'.join([name for name in varying if name in win.eqsys.par_names()])
    
    g=graphs.new(win,nPoints+1,win.cursor['nparts'],
                 varying,const,varVals,constVals,path)

                     
    g['mode']=mode
    g['callbacks']= {'success':lambda args:success(g,win,args),
                     'fail':lambda args:fail(g,win,args),
                     'cleanup':lambda:cleanup(g),
                     'minmax':lambda:minmax(g),
                     'cursor':lambda point:cursor(g,point,win)}

    
    identifier=create_string_buffer(bytes(path.encode("ascii")))
    varPars=create_string_buffer(bytes(varPars.encode("ascii")))
    
    eqsysID=win.eqsys.objectID
    if mode == 'map' and nSteps<0:
        eqsysID=win.invsys.objectID
        if win.invsys.var_names() != win.eqsys.var_names():
            raise Exception("map and inverse map have different variables")
        
    varWaveID=g['varwave'].objectID
    logID=win.log.objectID
    blobID=blb.objectID

    g['taskID']=lib.scigma_num_plot(identifier,eqsysID,logID,nSteps,
                                        win.cursor['nparts'],varWaveID,varPars,blobID,showall,noThread)

    g['cgraph']=gui.Bundle(win.glWindow,g['identifier'],g['npoints'],g['nparts'],
                           len(g['varying']),g['varwave'],g['constwave'],
                           lambda identifier, point: picking.select(identifier,point,win))
    g['cgraph'].set_marker_style(gui.POINT_TYPE[win.options['Drawing']['marker']['style'].label])
    g['cgraph'].set_marker_size(win.options['Drawing']['marker']['size'].value)
    g['cgraph'].set_point_style(gui.POINT_TYPE[win.options['Drawing']['point']['style'].label])
    g['cgraph'].set_point_size(win.options['Drawing']['point']['size'].value)
    g['cgraph'].set_style(gui.DRAWING_TYPE[win.options['Drawing']['style'].label])
    g['cgraph'].set_color(win.options['Drawing']['color'])
    g['cgraph'].set_delay(win.options['Drawing']['delay'].value)
    graphs.show(g,win)
    g['cgraph'].replay()
    
commands['p']=commands['pl']=commands['plo']=commands['plot']=plot 

def plotall(nSteps=1,path=None,win=None,noThread=False):
    plot(nSteps,path,win,True,noThread)

commands['p*']=commands['pl*']=commands['plo*']=commands['plot*']=plotall
    
def success(g,win,args):
    # finish the cpp Task
    lib.scigma_num_finish_task(g['taskID'])

    # if we plotted a stroboscopic map/ Poincare map, store the return time
    if g['mode']=='strobe' or g['mode']=='Poincare':
        nVarying=len(g['varying'])
        varWave=g['varwave']
        g['rtime']=varWave[-nVarying]-varWave[-2*nVarying]
        
    picking.select(g['identifier'],-1,win,True)
    g['cgraph'].finalize()

def fail(g,win,args):
    # finish the cpp Task
    lib.scigma_num_finish_task(g['taskID'])
    
def cleanup(g):
    pass

def minmax(g):
    g['min']={}
    g['max']={}
    mi=g['min' ]
    ma=g['max']
    varying=g['varying']
    varWave=g['varwave']
    rows=varWave.size()/len(varying)
    columns=len(varying)
    minima=[1e300]*columns
    maxima=[-1e300]*columns

    varWave.lock()
    d=varWave.data()
    for i in range(rows):
        for j in range(columns):
            value=d[i*columns+j]
            if value<minima[j]:
                minima[j]=value
            if value>maxima[j]:
                maxima[j]=value
    varWave.unlock()
    for i in range(columns):
        mi[varying[i]]=minima[i]
        ma[varying[i]]=maxima[i]
    constWave=g['constwave']
    const=g['const']
    for i in range(constWave.size()):
        mi[const[i]]=constWave[i]
        ma[const[i]]=constWave[i]

def click(g,x,y,point,win):
    pass

        
def cursor(g,point,win):
    
    nVarying=len(g['varying'])
    constVals=g['constwave'][:]
    names = ['t']+win.eqsys.var_names()+win.eqsys.par_names()
    
    if point >= 0:
        varVals=g['varwave'][nVarying*point:nVarying*(point+1)]
    else:
        nParts=g['nparts']
        nVarying=len(g['varying'])
        
        varVals=g['varwave'][-nParts*nVarying:]

    constVals=g['constwave'][:]

    return g['varying'], g['const'], varVals, constVals
        
def collect_varying_const(win,nParts,varying,const,varVals,constVals):
    # move time, variables and functions into varying, leave only the
    # rest in const
    r_varying=['t']+win.eqsys.var_names()+win.eqsys.func_names()
    r_const=const
    r_varVals=[]
    r_constVals=constVals

    # populate varyings defined by equation system
    # remove them from const, if necessary
    for i in range(len(r_varying)):
        name=r_varying[i]
        if name in varying:
            idx=varying.index(name)
            for j in range(nParts):
                r_varVals.insert(i+j+j*i,varVals[j*len(varying)+idx])
        else:
            idx=const.index(name)
            for j in range(nParts):
                r_varVals.insert(i+j+j*i,constVals[idx])
            del(r_const[idx])
            del(r_constVals[idx])

    # populate varyings defined by current cursor
    for i in range (len(varying)):
        name = varying[i]
        if name not in r_varying:
            for j in range(nParts):
                r_varVals.insert(len(r_varying)+j+len(r_varying)*j,varVals[j*len(varying)+i])
            r_varying.append(name)

    return r_varying, r_const, r_varVals, r_constVals

def plug(win=None):
    win = windowlist.fetch(win)
    # make sure that we do not load twice into the same window
    if not win.register_plugin('Iteration', lambda:unplug(win), commands):
        return
    
    # fill option panels
    win.glWindow.stall()
    panel=win.acquire_option_panel('Algorithms')
    enum = common.Enum({'non-stiff':0,'stiff':1},'stiff')
    panel.add('dt',1e-2)
    panel.add('odessa.type',enum)
    panel.add('odessa.atol',1e-9)
    panel.define('odessa.atol',"min=0.0")
    panel.add('odessa.rtol',1e-9)
    panel.define('odessa.rtol',"min=0.0")
    panel.add('odessa.mxiter',int(500))
    panel.define('odessa.mxiter',"min=1")
    enum = common.Enum({'numeric':0,'symbolic':1},'symbolic')
    panel.add('odessa.Jacobian',enum,True,"readonly=true")
    win.glWindow.flush()
    
def unplug(win=None):
    win = windowlist.fetch(win)
    # make sure that we do not unload twice from the same window
    if not win.unregister_plugin('Iteration'):
        return
    
    # remove options from panels
    win.glWindow.stall()
    panel=win.acquire_option_panel('Algorithms')
    panel.remove('dt')
    panel.remove('odessa.type')
    panel.remove('odessa.atol')
    panel.remove('odessa.rtol')
    panel.remove('odessa.mxiter')
    panel.remove('odessa.Jacobian')
    win.release_option_panel('Algorithms')
    win.glWindow.flush()
