from ctypes import *
from . import gui
from . import common
from . import library
from . import color
from . import windowlist

def on_entry_change(identifier,value,win):
    pass

def plug(win=None):
    win = windowlist.fetch(win)
    
    # make sure that we do not load twice into the same window
    if not win.register_plugin('style', lambda:unplug(win)):
        return

    # only redraw once after load is complete
    win.glWindow.stall()
    
    # add style option panel
    panel=win.acquire_option_panel('Style')
    panel.define('','iconified=true')
    panel.set_callback(lambda identifier,value: on_entry_change(identifier,value,win))
    panel.add('delay',common.Float(0.0))
    panel.add('sep1',gui.Separator())
    panel.add('color',color.from_string('red'))
    panel.define('color','colormode=hls');
    enum = common.Enum(gui.DRAWING_TYPE,'points')
    panel.add('style',enum)
    enum = common.Enum(gui.POINT_TYPE,'dot')
    panel.add('point.style',enum)
    panel.add('point.size',common.Float(16.0 if library.largeFontsFlag else 8.0))
    panel.define('point.size', 'min=1.0')
    panel.define('point.size', 'max=64.0')
    enum = common.Enum(gui.POINT_TYPE,'star')
    panel.add('marker.style',enum)
    panel.add('marker.size',common.Float(32.0 if library.largeFontsFlag else 16.0))
    panel.define('marker.size', 'min=1.0')
    panel.define('marker.size', 'max=64.0')
    panel.define('delay', 'min=0.0')
    panel.add('sep2',gui.Separator())
    enum = common.Enum({'dark':0,'light':1},'dark')
    panel.add('theme',enum)
    panel.add('GUI', True)    
    # redraw here
    win.glWindow.flush()
        
def unplug(win=None):
    win = windowlist.fetch(win)

    # make sure that we do not unload twice from the same window
    if not win.unregister_plugin('style'):
        return

    # redraw only once after unload is complete
    win.glWindow.stall()
    
    # remove options from panels
    panel=win.acquire_option_panel('Style')
    panel.remove('delay')
    panel.remove('sep1')
    panel.remove('color')
    panel.remove('style')
    panel.remove('point.style')
    panel.remove('point.size')
    panel.remove('marker.style')
    panel.remove('marker.size')
    panel.remove('sep2')
    panel.remove('theme')
    panel.remove('GUI')
    win.release_option_panel('Style')

    # redraw here
    win.glWindow.flush()
