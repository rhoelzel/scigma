import os, sys, inspect, ctypes
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
        execfile(filename)
        return
    try:
        with open(filename) as f:
            script = f.readlines()
        for line in script:
            try:
                win.on_console(line)
                # process notifications in between commands!
                win.loop_callback()
            except:
                pass # exceptions have already been caught in on_console()
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
