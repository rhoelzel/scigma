import os
from ctypes import *
from . import gui
from . import dat
from . import common
from . import lib
from . import windowlist
from . import graphs
from . import picking
from . import equations

commands={}

def set_auto_constants(blob):
    pass

def prepare_auto_folder(path):
    if not os.path.exists("."+path):
        os.makedirs("."+path)

lib.scigma_num_auto.restype=c_int
lib.scigma_num_auto.argtypes=[c_char_p,c_int,c_int,c_int,POINTER(c_int),c_int,c_int,c_bool]

def cont(nSteps=1, parameter=None, path=None,win=None,noThread=False):
    win = windowlist.fetch(win)

    try:
        nSteps = int(nSteps)
    except ValueError:
        raise Exception("error: could not read nSteps (usage: plot [nSteps] [name])")
    
    nVars=win.eqsys.n_vars()
    if nVars == 0:
        raise Exception ("error: no variables defined")
    if parameter not in win.eqsys.par_names():
        raise Exception("error: "+parameter+" is not a parameter")
    
    blob=common.Blob(win.options['AUTO'])
    set_auto_constants(blob);
    
    if not path:
        path=graphs.gen_ID("cont",win)
    prepare_auto_folder(path)

    cont={'identifier': 'path', 'npoints':nSteps,
          'br':0,'fp':0, 'bp':0, 'hb':0}
    cont['varying']=win.eqsys.var_names()+[parameter]
    cont['const']=win.eqsys.par_names().remove(parameter)
    cont['callbacks']={'success':lambda args:success(cont,win,args),
                       'fail':None,
                       'cleanup':None,
                       'minmax':None,
                       'cursor':None}
    
    common.dict_enter(path,win.graphs,cont)

    identifier=create_string_buffer(bytes(path.encode("ascii")))
    C_IntArrayType=c_int*1
    indexArray=C_IntArrayType(win.eqsys.par_names().index(parameter))

    cwd = os.getcwd()
    os.chdir("."+path)
    taskID=lib.scigma_num_auto(identifier,win.eqsys.objectID,win.log.objectID,
                               nSteps,indexArray,1,blob.objectID,True)
    os.chdir(cwd)

commands['c']=commands['co']=commands['con']=commands['cont']=cont

def success(cont,win,args):
    # finish the cpp Task
    #lib.scigma_num_finish_task(g['taskID'])

    if len(args)<0:
    # create the visible object        
        pointStyle= gui.RING
        pointSize = win.options['Drawing']['marker']['size'].value
        color = win.options['Drawing']['color']
        
        cont['br']=cont['br']+1
        g={'identifier':cont['identifier']+'.'+str(cont['br'])}
        g['cgraph']=gui.Bundle(win.glWindow,cont['identifier']+'.'+str(cont['br']),
                               cont['npoints'],1,len(cont['varying']),
                               dat.Wave(objectID=int(args[0])),dat.Wave(objectID=int(args[0])),
                               lambda identifier, point: picking.select(identifier,point,win))
        g['cgraph'].set_point_style(pointStyle)
        g['cgraph'].set_point_size(pointSize)
        g['cgraph'].set_color(color)
        g['cgraph'].finalize()

        common.dict_enter(cont['identifier']+'.'+str(cont['br']),win.graphs,g)
        
        graphs.show(g,win)

    #picking.select(g['identifier'],-1,win)
    #g['cgraph'].finalize()

def plug(win=None):
    win = windowlist.fetch(win)
    # make sure that we do not load twice into the same window
    if not win.register_plugin('Continuation', lambda:unplug(win), commands):
        return
    
    # fill option panels
    win.glWindow.stall()
    panel=win.acquire_option_panel('AUTO')
    panel.add('NPR',0)
    panel.add('stepsize.ds', 0.01)
    panel.add('stepsize.dsmin', 1e-6)
    panel.add('stepsize.dsmax', 1e0)
    panel.add('tolerances.epsl',1e-7)
    panel.add('tolerances.epsu',1e-7)
    panel.add('tolerances.epss',1e-6)
    panel.add('bounds.rl0',-1e300)
    panel.add('bounds.rl1',1e300)
    panel.add('bounds.a0',-1e300)
    panel.add('bounds.a1',1e300)
    panel.add('advanced AUTO.NTST',100)
    panel.add('advanced AUTO.NCOL',4)
    panel.add('advanced AUTO.IAD',3)
    panel.add('advanced AUTO.IADS',1)
    panel.add('advanced AUTO.ITMX',8)
    panel.add('advanced AUTO.NWTN',3)
    panel.add('advanced AUTO.ITNW',5)
    panel.add('advanced AUTO.NMX',0)
    panel.add('advanced AUTO.ILP',1)
    panel.add('advanced AUTO.ISP',2)
    panel.add('advanced AUTO.ISW',1)
    panel.add('advanced AUTO.MXBF',1)
    panel.add('advanced AUTO.IPS',1)
    panel.add('advanced AUTO.IIS',3)
    panel.add('advanced AUTO.IID',3)
    panel.add('advanced AUTO.IPLT',0)
    panel.define('advanced AUTO', 'opened=false')
    panel.add('HOMCONT.NUNSTAB',-1)
    panel.add('HOMCONT.NSTAB',-1)
    panel.add('HOMCONT.IEQUIB',1)
    panel.add('HOMCONT.ITWIST',0)
    panel.add('HOMCONT.ISTART',5)
    panel.define('HOMCONT', 'opened=false')

    win.glWindow.flush()
    
def unplug(win=None):
    win = windowlist.fetch(win)
    # make sure that we do not unload twice from the same window
    if not win.unregister_plugin('Continuation'):
        return
    
    # remove options from panels
    win.glWindow.stall()
    panel=win.acquire_option_panel('AUTO')
    panel.remove('NPR')
    panel.remove('stepsize.ds')
    panel.remove('stepsize.dsmin')
    panel.remove('stepsize.dsmax')
    panel.remove('tolerances.epsl')
    panel.remove('tolerances.epsu')
    panel.remove('tolerances.epss')
    panel.remove('bounds.rl0')
    panel.remove('bounds.rl1')
    panel.remove('bounds.a0')
    panel.remove('bounds.a1')
    panel.remove('advanced AUTO.NTST')
    panel.remove('advanced AUTO.NCOL')
    panel.remove('advanced AUTO.IAD')
    panel.remove('advanced AUTO.IADS')
    panel.remove('advanced AUTO.ITMX')
    panel.remove('advanced AUTO.NWTN')
    panel.remove('advanced AUTO.ITNW')
    panel.remove('advanced AUTO.NMX')
    panel.remove('advanced AUTO.ILP')
    panel.remove('advanced AUTO.ISP')
    panel.remove('advanced AUTO.ISW')
    panel.remove('advanced AUTO.MXBF')
    panel.remove('advanced AUTO.IPS')
    panel.remove('advanced AUTO.IIS')
    panel.remove('advanced AUTO.IID')
    panel.remove('advanced AUTO.IPLT')
    panel.remove('HOMCONT.NUNSTAB')
    panel.remove('HOMCONT.NSTAB')
    panel.remove('HOMCONT.IEQUIB')
    panel.remove('HOMCONT.ITWIST')
    panel.remove('HOMCONT.ISTART')

    win.release_option_panel('Algorithms')
    win.glWindow.flush()
