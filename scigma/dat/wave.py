from ctypes import *
from .. import lib

class Wave(object):
    """ Wrapper for Wave objects """

    lib.scigma_dat_create_wave.argtypes=[c_int,c_int, POINTER(c_double),c_int]
    lib.scigma_dat_wave_push_back.argtypes=[c_int,POINTER(c_double),c_int]
    lib.scigma_dat_wave_data.restype=POINTER(c_double)

    def __init__(self, values=None, columns = None, rows = 0x1000):
        if not values:
            cValues=None
            nValues=0
            if not columns:
                columns=0
        else:
            C_DoubleArrayType=c_double*len(values)
            cValuesArray=C_DoubleArrayType(*values)
            cValues=cast(cValuesArray,POINTER(c_double))
            nValues=len(values)
            if not columns:
                columns=nValues
        self.objectID = lib.scigma_dat_create_wave(rows,columns,cValues,nValues)

    def destroy(self):
        lib.scigma_dat_destroy_wave(self.objectID)

    def __str__(self):
        return 'Wave('+str(self.columns())+'x'+str(self.rows())+')'

    def __repr__(self):
        return self.__str__()

    def push_back(self, values):
        C_DoubleArrayType=c_double*len(values)
        cValuesArray=C_DoubleArrayType(*values)
        cValues=cast(cValuesArray,POINTER(c_double))
        nValues=len(values)
        lib.scigma_dat_wave_push_back(self.objectID, cValues,nValues)

    def push_back(self, nValues):
        lib.scigma_dat_wave_pop_back(self.objectID,nValues)
        
    def lock(self):
        lib.scigma_dat_wave_lock(self.objectID)

    def unlock(self):
        lib.scigma_dat_wave_unlock(self.objectID)

    def size(self):
        return lib.scigma_dat_wave_size(self.objectID)

    def data(self):
        return lib.scigma_dat_wave_data(self.objectID)

    def __getitem__(self, key):
        length=self.size()
        if isinstance(key, slice):
            self.lock()
            data=self.data()
            result = [data[i] for i in xrange(*key.indices(length))]
            self.unlock()
        elif isinstance( key, int ) :
            if key < 0 : #Handle negative indices
                key += length
            if key >= length or key <0:
                raise IndexError, "The index (%d) is out of range."%key
            self.lock()
            result=self.data()[key]
            self.unlock()
        else:
            raise TypeError, "Invalid argument type."
        return result;
