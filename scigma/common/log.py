from ctypes import *
from .. import lib

class Log(object):
    """ Wrapper for Log objects """
    
    lib.scigma_common_log_pop.restype=c_char_p

    SUCCESS=0
    FAIL=1
    DATA=2
    WARNING=3
    ERROR=4
    DEFAULT=5
    
    def __init__(self):
        self.objectID=lib.scigma_common_create_log()
        
    def destroy(self):
        lib.scigma_common_destroy_log(self.objectID)
        
    def pop(self,logType):
        retval=lib.scigma_log_pop(self.objectID, logType)
        if not retval:
            return ""
        else:
            return str(retval.decode())

                                                                        
