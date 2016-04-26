from . import gui
from . import common
from . import windowlist
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
        period=float(eqsys.parse(win.equationPanel.get('period')))
        if period<0:
            win.console.write_warning("period = "+str(period)+" is negative; using absolute value instead") 
            period = -period
        blob.set("period", period)
    if mode=='Poincare':
        secvar=options.get(['secvar'],win).strip()
        secidx=eqsys.variable_names().index(secvar)
        blob.set("secidx", secidx)

    return blob

def plug(win=None):
    win = windowlist.fetch(win)
    # make sure that we do not load twice into the same window
    if not win.register_plugin('Iteration', lambda:unplug(win), commands):
        return
    
    # fill option panels
    win.glWindow.stall()
    panel=win.acquire_option_panel('Algorithms')
    enum = common.Enum({'non-stiff':0,'stiff':1},'stiff')
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
    panel.remove('odessa.type')
    panel.remove('odessa.atol')
    panel.remove('odessa.rtol')
    panel.remove('odessa.mxiter')
    panel.remove('odessa.Jacobian')
    win.release_option_panel('Algorithms')
    win.glWindow.flush()
