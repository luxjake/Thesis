#!/usr/bin/python3

import os
import sys

def nlines_file(f):
    tmp = open(f, 'r')
    i = 0
    for l in tmp:
        i += 1
    tmp.close()
    return i


def main():

    if len(sys.argv) != 2:
        print("1 argument needed : a file name containing the versions directory in order")
        sys.exit()
        
    ver = []

    verfile = open(sys.argv[1])

    fileext = ""

    for v in verfile:
        v = v.rstrip()
        if ('file type : ' in v) and (fileext==""):
            v = v.replace('file type : ', '')
            fileext = v
        else:
            ver.append(v)
    
    verfile.close()

    fpath = {}
    vf = {}

    for v in ver:
        os.chdir(v)
        for root, dirs, files in os.walk("."):
            for f in files:
                if fileext in f:
                    if f not in vf:
                        vf[f] = []
                        vf[f].append(v)
                    else:
                        if v not in vf[f]:
                            vf[f].append(v)
                    if (f,v) not in fpath:
                        p = os.path.join(root, f)
                        p = p[1:]
                        p = v + p
                        fpath[(f,v)] = p
        
        os.chdir('..')

    of = open('./conf.txt', 'w')
        
    for k in vf:
        for v in ver:
            if v not in vf[k]:
                fn = v + '/' + k
                print(fn + ' does not exist. We create an empty replacement.')
                tmpf = open(fn, 'w')
                tmpf.write('dummy')
                tmpf.close()
                ind = ver.index(v)
                vf[k].insert(ind, v)
                fpath[(k,v)] = fn
            
        for i in range(1,len(vf[k])):
            pv = vf[k][i-1]
            cv = vf[k][i]
            versions = pv.replace('/', '') + '_' + cv.replace('/', '')
            resfile = versions + '_' + k
            s = fpath[(k,pv)] + ' ' + fpath[(k,cv)] + ' ' + resfile + ' ' + versions + '\n'
            of.write(s)    
        
    of.close()
    
    
if __name__ == "__main__":
    main()
