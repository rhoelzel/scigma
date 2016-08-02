from ctypes import *
from . import gui
from . import common
from . import lib
from . import library
from . import windowlist
from . import graphs
from . import picking
from . import equations

commands={}

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
