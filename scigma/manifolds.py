from ctypes import *
from . import gui
from . import common
from . import lib
from . import library
from . import windowlist
from . import graphs
from . import picking
from . import equations
from . import iteration

commands={}
"""
def manifold(stable,nSteps,g=None,path=None,win=None,showall=False):
    win = windowlist.fetch(win)

    try:
        nSteps = int(nSteps)
    except ValueError:
        raise Exception("error: could not read nSteps (usage: mu/ms <nSteps> [name])")

    if nSteps<0:
        nSteps=-nSteps

    g=graphs.get(g,win)

    # see if we can use the data in g to create an invariant manifold
    try:
        evreal = g['evreal']
        evimag = g['evimag']
        evecs = g['evecs']
    except:
        raise Exception(g['identifier']+': point has no eigenvalue and/or eigenvector data')

    picking.select(g['identifier'],-1,win,True)

    blob=iteration.blob(win)
    if not path:
        path=graphs.gen_ID("mf",win)
    
    evindex=instance.options['Numerical']['manifolds']['evec1']

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
                                                                                                                                                                                                                                                                               lambda identifier,point,ctrl:instance.select_callback(identifier,point,ctrl))
                                                                                                                                                                                                                                                                objects.show(obj,instance)

                                                                                                                                                                                                                                                                    instance.pendingTasks=instance.pendingTasks+len(objlist)
                                                                                                                                                                                                                                                                        return objlist

def munstable(n=1,origin=None,name=None,instance=None,noThread=False):
"""
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
"""            return manifold(False,n,origin,name,instance,False,noThread)
        commands['mu']=commands['mun']=commands['muns']=commands['munst']=commands['munsta']=commands['munstab']=commands['munstabl']=commands['mu\
nstable']=munstable
        def mstable(n=1,origin=None,name=None,instance=None,noThread=False):
                return manifold(True,n,origin,name,instance,False,noThread)
            commands['ms']=commands['mst']=commands['msta']=commands['mstab']=commands['mstabl']=commands['mstable']=mstable
            def munstableall(n=1,origin=None,name=None,instance=None,noThread=False):
                    return manifold(False,n,origin,name,instance,True,noThread)
                commands['mu*']=commands['mun*']=commands['muns*']=commands['munst*']=commands['munsta*']=commands['munstab*']=commands['munstabl*']=al\
                              ias['munstable*']=munstableall
                def mstableall(n=1,origin=None,name=None,instance=None,noThread=False):
                        return manifold(True,n,origin,name,instance,True,noThread)
                    commands['ms*']=commands['mst*']=commands['msta*']=commands['mstab*']=commands['mstabl*']=commands['mstable*']=mstableall
"""

def plug(win=None):
    win = windowlist.fetch(win)
    # make sure that we do not load twice into the same window
    if not win.register_plugin('Manifolds', lambda:unplug(win), commands):
        return
    
    # fill option panels
    win.glWindow.stall()
    panel=win.acquire_option_panel('Algorithms')
    panel.add('manifolds.eps',1e-5)
    panel.add('manifolds.ds',0.1)
    panel.add('manifolds.alpha',0.3)
    win.glWindow.flush()
    
def unplug(win=None):
    win = windowlist.fetch(win)
    # make sure that we do not unload twice from the same window
    if not win.unregister_plugin('Manifolds'):
        return
    
    # remove options from panels
    win.glWindow.stall()
    panel=win.acquire_option_panel('Algorithms')
    panel.remove('manifolds.eps')
    panel.remove('manifolds.ds')
    panel.remove('manifolds.alpha')
    win.release_option_panel('Algorithms')
    win.glWindow.flush()
