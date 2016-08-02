import re, os
from common import Log
from . import num
from . import gui
from . import common
from . import library
from . import options
from . import equations
from . import picking
from . import graphs
from .windowlist import windows

class Window(object):
    """ Manages a single Scigma window.
    
    """
    
    def __init__(self):
        windows.append(self)
        self.log=common.Log()
        self.looplambda=lambda:self.loop_callback()
        gui.add_loop_callback(self.looplambda)
        
        self.glWindow=gui.GLWindow()
        self.console=gui.Console(self.glWindow,library.largeFontsFlag)
        self.glWindow.connect(self.console)
        self.console.set_callback(lambda line: self.on_console(line))

        self.unplugFunctions={}
        
        self.options={}
        self.optionPanels={}
        self.commands={}
        
    def destroy(self):
        for key in self.unplugFunctions.keys():
            self.unplugFunctions[key]()
            
        for key in self.optionPanels.keys():
            self.optionPanels[key].destroy()
            
        self.glWindow.destroy()
        gui.remove_loop_callback(self.looplambda)
        self.log.destroy()
        windows.remove(self)
        
    def register_plugin(self, name, funplug, commands=None):
        if self.has_plugin(name):
            return False
        self.unplugFunctions[name]=funplug
        self.commands[name]=commands
        return True

    def unregister_plugin(self, name):
        if not self.has_plugin(name):
            return False
        del self.unplugFunctions[name]
        del self.commands[name]
        return True

    def has_plugin(self, name):
        return name in self.unplugFunctions
    
    def acquire_option_panel(self, identifier):
        if not identifier in self.options:
            self.optionPanels[identifier]=gui.ATWPanel(self.glWindow,identifier)
            self.options[identifier]=self.optionPanels[identifier].data
            self.glWindow.request_redraw()
        return self.optionPanels[identifier] 

    def release_option_panel(self, identifier):
        if not self.options[identifier]:
            del self.options[identifier]
            self.optionPanels[identifier].destroy()
            del self.optionPanels[identifier]
            self.glWindow.request_redraw()


    def loop_callback(self):
        mtype, message=self.log.pop()
        while message is not "":
            if mtype==Log.DATA:
                self.console.write_data(message)
            elif mtype==Log.WARNING:
                self.console.write_warning(message)
            elif mtype==Log.ERROR:
                self.console.write_error(message)
            elif mtype==Log.SUCCESS:
                args=message.split("|")
                graphs.success(args[0],args[1:],self)
            elif mtype==Log.FAIL:
                args=message.split("|")
                graphs.fail(args[0],args[1:],self)
            else:
                self.console.write(message)
            mtype,message=self.log.pop()

    def on_console(self,line):
        line=line.partition('#')[0]         # remove any comment
        line=re.sub("\s+=\s+","=",line)     # turn x = y into x=y (avoids interpretation as x('=','y'))
        list=line.split()                   # separate command and arguments
        if len(list)==0:
            return
        cmd=list[0]
        args=list[1:]

        paths=[]
        common.dict_full_paths(cmd,self.commands,paths) # search for command in dictionary

        if len(paths) is 0: # this is possibly an equation or the attempt to set/query an option
            try:
                equations.parse(line,self)
                picking.on_parse(self)
            except Exception as e: # if parsing fails, check if we are trying to set/query option
                try:
                    if len(args)==0:
                        result=options.get_string(cmd,self)
                        self.console.write_data(result+"\n")
                    else:
                        options.set(cmd,' '.join(args),self)
                except Exception as ee:
                    if ee.args[0][:9] == 'ambiguous':
                        error=ee.args[0]
                    elif e.args[0]=='syntax error':
                        # a generic failure in parse() suggests that we were not
                        # trying to parse an equation, therefore forward ee as well
                        error =ee.args[0]+', or syntax error'
                    else:
                        # if the input was sufficiently well formed to trigger a
                        # specific parser warning, assume that no set/query option was tried
                        # and ignore ee
                        error=e.args[0]
                    
                    self.console.write_error(error+'\n')
                    raise Exception(error)
                   
        elif not len(paths) is 1: # there is more than one command with the same name or path
            error = "ambiguous command; use qualified name, e.g. " + paths[0] + " or " + paths[1]
            self.console.write_error(error+'\n')
            raise Exception(error)
        else: # run command with the arguments
            try:
                cfunc=common.dict_entry(paths[0],self.commands)
                cfunc(*args,win=self)
            except Exception as e:
                self.console.write_error(str(e.args[0])+'\n')
                raise e

            
