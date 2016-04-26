import re
from ctypes import *
from time import time
from . import gui
from . import dat
from . import common
from . import equations
from . import windowlist
from . import lib

commands={}
independentVariables={}

def show(g, win=None):
    win=windowlist.fetch(win)
    g=get(g,win)
    if not g['__visible__']:
        g['__visible__']=True
        set_view(g,independentVariables[win])
        win.glWindow.add_drawable(g['__cgraph__'])        

commands['show']=show
        
def hide(g, win=None):
    win=windowlist.fetch(win)
    g=get(g,win)
    if g['__visible__']:
        g['__visible__']=False
        win.glWindow.remove_drawable(g['__cgraph__'])

commands['hide']=hide

def delete(identifier, win=None):
    win=windowlist.fetch(win)
    path=None
    # this deletes named graph, or a node with all graphs below that node
    path=common.dict_single_path(identifier,win.graphs,'graph',lambda entry: '__fcleanup__' not in entry)
    entry=common.dict_entry(path,win.graphs)
    glist=[]
    common.dict_leaves(entry,glist, lambda entry: '__fcleanup__' not in entry)
    for g in glist:
        destroy(g,win)
    parts=path.rpartition('.')
    parent=common.dict_entry(parts[0],win.graphs)
    child=parts[2]
    del parent[child]

def get(g, win):
    if isinstance(g,str):
        return common.dict_single_entry(g,win.graphs,'graph',lambda entry: '__fcleanup__' not in entry)
    else:
        return g

def get_all(win):
    def g(dictionary):
        graph_keys = [key for key in dictionary if isinstance(dictionary[key],dict) and '__fcleanup__' in dictionary[key]]
        other_keys = [key for key in dictionary if isinstance(dictionary[key],dict) and not '__fcleanup__' in dictionary[key]]
        return [dictionary[key] for key in graph_keys]+[graph for key in other_keys for graph in g(dictionary[key])]
    return g(win.graphs)
    
def destroy(g, win):
    if g['__visible__']:
        win.glWindow.remove_drawable(g['__cgraph__'])

    if g['__cgraph__']:
        g['__cgraph__'].destroy()

    g['__mesh__'].destroy()
    g['__varwave__'].destroy()
    g['__constwave__'].destroy()

    if g['__fcleanup__']:
        g['__fcleanup__'](g,win)

def gen_ID(prefix,win):
    """generates a unique name with given prefix
    
    This generates a name of the form 'TR_00001' (if prefix is 'TR').
    The counter is increased for each prefix, to ensure that all 
    names are unique (as long as no more than 100000 graphs have
    the same prefix)
    """
    if prefix not in win.graphIDs:
        win.graphIDs[prefix]=1
    count = win.graphIDs[prefix]
    ID = prefix + str(count).zfill(7)
    win.graphIDs[prefix]=count+1
    return ID

def new(win,path=None,nPoints=1,nParts=1,meshVals=None,varying=None,const=None,varVals=None,constVals=None, finit=None, fcleanup=None):
    """ adds a new graph to the window 
    """    

    if meshVals is None:
        meshVals=range(nParts)

    # use current point if no data given
    if varying is None and const is None and varVals is None and constVals is None:
        const, constVals = equations.point(win)
        varying=[]
        varVals=[]
        
    g={'__varying__':varying,'__const__':const,'__visible__':False,'__npoints__':nPoints,
       '__nparts__':nParts,'__finit__': finit,'__fcleanup__':fcleanup,'__cgraph__':None}

    if path:
        common.dict_enter(path,win.graphs,g)    
          
    # only initizialize Wave here, in case something goes wrong
    # with the new dict entry
    g['__mesh__']=dat.Mesh(meshVals,columns=1,rows=nPoints*nParts)
    g['__varwave__']=dat.Wave(varVals,columns=len(varying),rows=nPoints*nParts)
    g['__constwave__']=dat.Wave(constVals,rows=1)
    
    return g


GLSL_BUILT_IN_FUNCTIONS=["sin","cos","tan","asin","acos","atan","abs",
                           "mod","sign","step","pow","exp","log","sqrt"]

def is_variable(x):
    if x[0]>'0'and x[0]<='9':
        return False
    if x in GLSL_BUILT_IN_FUNCTIONS:
        return False
    return True

def coordinate_expressions(win):
    expressions=[]
    for i in range(gui.N_COORDINATES):
        expressions.append(win.options['View'][gui.COORDINATE_NAME[i]])

    # if an expression is empty, assume that it's value is zero, except for
    # colors (where an empty expression string means that no color map is used)
    axes=win.options['View']['axes'].label
    for i in range(gui.N_COORDINATES):
        if gui.VIEW_TYPE[axes]&gui.COORDINATE_FLAG[gui.COORDINATE_NAME[i]]:
            if expressions[i]=='':
                expressions[i]='0.0'
        else:
            if i==gui.C_INDEX:
                expressions[i]=''
            else:
                expressions[i]='0.0'
    return expressions

def independent_variables(expressions):
    skip = (expressions[gui.C_INDEX]=='')
    variables=[]
    for j in range(gui.N_COORDINATES):
        if j==gui.C_INDEX and skip:
            continue
        exprlist = re.sub('[^0-9_.a-zA-Z]+', ' ', expressions[j]).split()   
        for expr in exprlist:
            try:
                x=float(expr)
            except:
                if is_variable(expr):
                    variables.append(expr)
    return sorted(list(set(variables)))

def set_view(g, expBuffer,indBuffer,timeStamp,independentVariables):
    indices=[]
    varying=g['__varying__']
    const=g['__const__']
    for var in independentVariables:
        try: 
            index=varying.index(var)
            indices.append(index+1)
        except:
            try:
                index=const.index(var)
                indices.append(-index-1)
            except:
                indices.append(0)
                
    g['__cgraph__'].set_view(indices,expBuffer,indBuffer,timeStamp)

def on_view_change(win):
    expressions=coordinate_expressions(win)
    independentVariables[win]=independent_variables(expressions)

    arg1=''
    for x in expressions:
        arg1+='|'+x
    arg1=bytes(arg1.strip('|').encode("ascii"))
    
    arg2=''
    for x in independentVariables[win]:
        arg2+='|'+x
    arg2=bytes(arg2.strip('|').encode("ascii"))
    
    
    glist=[]
    common.dict_leaves(win.graphs,glist,lambda entry: '__fcleanup__' not in entry)

    timeStamp=time()
    expBuffer=create_string_buffer(arg1)
    indBuffer=create_string_buffer(arg2)
    
    for g in glist:
        set_view(g,expBuffer,indBuffer,independentVariables[win],timeStamp)

    if win.has_plugin('picking'):
        set_view(win.cursor,expBuffer,indBuffer,independentVariables[win],timeStamp)
        
    win.glWindow.request_redraw()


def min_max(g):
    g['__min__']={}
    g['__max__']={}
    mi=g['__min__']
    ma=g['__max__']
    varWave = g['__varwave__']
    d=varWave.data()
    rows=varWave.rows()
    columns=varWave.columns()
    minima=[1e300]*columns
    maxima=[-1e300]*columns
    
    for i in range(rows):
        for j in range(columns):
            value=d[i*columns+j]
            if value<minima[j]:
                minima[j]=value
            if value>maxima[j]:
                maxima[j]=value
    varying=g['__varying__']
    for i in range(columns):
        mi[varying[i]]=minima[i]
        ma[varying[i]]=maxima[i]
    constWave=g['__constwave__']
    const=g['__const__']
    for i in range(constWave.columns()):
        mi[const[i]]=constWave[i]
        ma[const[i]]=constWave[i]

def fail(identifier, win):
    g=get(identifier,win)
    # the next two lines are needed to stop drawing if
    # delay is set and the computation ended prematurely
    g['__cgraph__'].set_n_points(0)
    win.glWindow.request_redraw()
    
    delete(identifier,win)
    
def success(identifier, win):
    g=get(identifier,win)
    # get minima and maxima
    min_max(g)

    # perform any other initialization tasks
    if g['__finit__']:
        g['__finit__'](identifier,win)

    lib.scigma_num_destroy_thread(g['__thread__'])
    g['__thread__']=None

def plug(win):
    if not win.register_plugin('graphs', lambda:unplug(win),commands):
        return
    setattr(win,'graphs',{})
    setattr(win,'graphIDs',{})

    independentVariables[win]=None
    
    #initial build of GL shaders occurs here
    on_view_change(win)
    
def unplug(win):
    if not win.unregister_plugin('graphs'):
        return
    delattr(win,'graphs')
    delattr(win,'graphIDs')

    del independentVariables[win]

    
