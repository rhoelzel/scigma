from ctypes import *
from .. import lib

class Mesh(object):
    """ Wrapper for Mesh objects """

    lib.scigma_dat_create_mesh.argtypes=[c_int,c_int, POINTER(c_double)]

    def __init__(self, nDim, initial):
        C_DoubleArrayType=c_double*len(initial)
        cValuesArray=C_DoubleArrayType(initial)
        cValues=cast(cValuesArray,POINTER(c_double))
        nInitial=len(initial)/nDim
        self.objectID = lib.scigma_dat_create_mesh(nDim,nInitial,cValues)

    def destroy(self):
        lib.scigma_dat_destroy_mesh(self.objectID)

    def __str__(self):
        return 'Mesh('+str(self.columns())+'x'+str(self.rows())+')'

    def __repr__(self):
        return self.__str__()

