import os, sys, inspect, ctypes

""" Load shared library:"""
if sys.platform=='linux2':
    sep='/'
    libfile="libscigma.so"
elif sys.platform=='darwin':
    sep='/'
    libfile="libscigma.dylib"
elif sys.platform=='win32':
    sep='\\'
    libfile="libscigma.dll"
else:
    quit()

libpath=inspect.stack()[0][1].rpartition(sep)[0]
path=os.getcwd()
try:
    os.chdir(libpath)
    lib=ctypes.CDLL(libfile)
    os.chdir(path)
except:
    os.chdir(path)
    lib=ctypes.CDLL(libpath+sep+libfile)

largeFontsFlag=False
ctypes.c_bool.in_dll(lib,"LARGE_FONTS_FLAG").value=largeFontsFlag

