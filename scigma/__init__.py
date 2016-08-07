import os, sys, inspect, ctypes
try:
    if sys.version_info.major == 2:
        # We are using Python 2.x
        import tkFileDialog as tkfile
    elif sys.version_info.major == 3:
        # We are using Python 3.x
        import tkinter.filedialog as tkfile
except:
        print("tkinter not found / not using tk")

from .library import lib
from . import options
from . import graphs
from . import equations
from . import view
from . import style
from . import iteration
from . import guessing
from . import manifolds
from . import continuation
from . import window
from . import windowlist

def new(win=None):
    w=window.Window()

    #initialize global commands
    w.commands['n']=w.commands['ne']=w.commands['new']=new    
    w.commands['l']=w.commands['lo']=w.commands['loa']=w.commands['load']=load
    setattr(w,'script','none')
    setattr(w,'source','none')
    w.commands['q']=w.commands['qu']=w.commands['qui']=w.commands['quit']=w.commands['end']=w.commands['bye']=bye

    panel=w.acquire_option_panel('Global')
    panel.define('','iconified=true')
    enum = common.Enum({'off':0,'on':1},'off')
    panel.add('echo',enum)
    enum = common.Enum({'off':0,'on':1},'on')
    panel.add('threads',enum)
    
    #initialize plugins
    equations.plug(w)
    view.plug(w)
    style.plug(w)
    graphs.plug(w)
    picking.plug(w)
    iteration.plug(w)
    guessing.plug(w)
    manifolds.plug(w)
    continuation.plug(w)
    return w

def load(filename=None,win=None):
    """ load <filename>

    Loads a script file and executes the content;
    opens a file dialog if filename is not given.
    """
    win=windowlist.fetch(win)

    if not filename:
        raise Exception("no filename specified")
    if filename[-2:]=='py':
        if sys.version_info.major==2:
            execfile(filename)
        elif sys.version_info.major==3:
            with open(filename) as f:
                code = compile(f.read(), filename, 'exec')
                exec(code, globals(), locals())
        return
   
    try:
        with open(filename) as f:
            script = f.readlines()
        q=[line.strip() for line in script]
        threads=win.options['Global']['threads'].label
        win.queue=(['threads off']
                   +q
                   +['threads on'] if threads=='on' else []
                   +win.queue)
    except IOError:
        raise Exception(filename+": file not found")
    win.script=filename
    win.glWindow.set_title("SCIGMA - script: "+win.script+" - equations: "+win.source)

def bye(win=None):
    """ quit

    closes the window and frees all resources
    """
    win=windowlist.fetch(win)

#    clear(win)
    win.destroy()
    

cwd = os.getcwd()

# create main window
main=new(None)

# do not know why this is necessary,
# but on my machine cwd gets altered
# during the first ctypes call
os.chdir(cwd)
