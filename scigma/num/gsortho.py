import math

def dot(v1,v2):
    result=0
    for i in range(len(v1)):
        result+=v1[i]*v2[i]
    return result
    
def gs_coefficient(v1, v2):
    return dot(v2, v1)/ dot(v1, v1)

def multiply(coefficient, v):
    return map((lambda x : x * coefficient), v)

def proj(v1, v2):
    return multiply(gs_coefficient(v1, v2) , v1)

def gsortho(X):
    Y = []
    for i in range(len(X)):
        temp_vec = X[i]
        for inY in Y :
            proj_vec = proj(inY, X[i])
            temp_vec = map(lambda x, y : x - y, temp_vec, proj_vec)
        Y.append(temp_vec)
    for i in range(len(X)):
        l = math.sqrt(dot(Y[i],Y[i]))
        for j in range(len(Y[i])):
            Y[i][j]=Y[i][j]/l
    return Y

