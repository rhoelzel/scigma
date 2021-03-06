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
    w.commands['write']=write
    w.commands['data']=data
    w.commands['warn']=warn
    w.commands['comment']=comment
    w.commands['error']=error
    w.commands['writeln']=writeln
    w.commands['dataln']=dataln
    w.commands['warnln']=warnln
    w.commands['commentln']=commentln
    w.commands['errorln']=errorln
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
    sweeping.plug(w)
    continuation.plug(w)
    return w

def write(string, win=None):
    win=windowlist.fetch(win)
    win.console.write(string)

def data(string, win=None):
    win=windowlist.fetch(win)
    win.console.write_data(string)

def warn(string, win=None):
    win=windowlist.fetch(win)
    win.console.write_warning(string)

def comment(string, win=None):
    win=windowlist.fetch(win)
    win.console.write_comment(string)

def error(string, win=None):
    win=windowlist.fetch(win)
    win.console.write_error(string)

def writeln(string, win=None):
    win=windowlist.fetch(win)
    win.console.write(string+'\n')

def dataln(string, win=None):
    win=windowlist.fetch(win)
    win.console.write_data(string+'\n')

def warnln(string, win=None):
    win=windowlist.fetch(win)
    win.console.write_warning(string+'\n')

def commentln(string, win=None):
    win=windowlist.fetch(win)
    win.console.write_comment(string+'\n')

def errorln(string, win=None):
    win=windowlist.fetch(win)
    win.console.write_error(string+'\n')
    
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
                   +(['threads on'] if threads=='on' else [])
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
