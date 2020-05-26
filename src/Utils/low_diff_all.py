#!/usr/bin/python3

import sys
import os
import subprocess
import time
import glob
import shutil
from scipy.stats.mstats import gmean

if len(sys.argv) != 3:
    print("2 argument needed :\n- a file name containing a list of pairs of files to compare\n- the name of the result file")
    sys.exit()

param_file = open(sys.argv[1], 'r')

vres = {}

pairs = []
for l in param_file:
    if l[0] != "#":
        l = l.rstrip()
        line = l.split(' ')
        pairs.append((line[0],line[1],line[2],line[3],line[4]))
        vres[line[4]] = {}
        vres[line[4]]['added'] = []
        vres[line[4]]['deleted'] = []
        vres[line[4]]['ntriples1'] = []
        vres[line[4]]['ntriples2'] = []
        vres[line[4]]['padds'] = []
        vres[line[4]]['pdels'] = []
        vres[line[4]]['union'] = []

for p in pairs:
    if not os.path.isfile(p[0]):
        print('The file ' + p[0] + ' does not exist.')
        sys.exit()
    if not os.path.isfile(p[1]):
        print('The file ' + p[1] + ' does not exist.')
        sys.exit()
    
    print('Diff for files : ' + p[0] + ' & ' + p[1])
    subprocess.run(['lowdiff', p[0], p[1], p[2], p[3]])
        
    tmpf = open(p[3] + '.res', 'r')
    tmpf.readline()
    tmpf.readline()
    l = tmpf.readline()
    sl = l.split(' ')
    vres[p[4]]['added'].append(float(sl[0]))
    vres[p[4]]['deleted'].append(float(sl[1]))
    vres[p[4]]['ntriples2'].append(float(sl[2]))
    vres[p[4]]['ntriples1'].append(float(sl[3]))
    vres[p[4]]['padds'].append(float(sl[4]))
    vres[p[4]]['pdels'].append(float(sl[5]))
    vres[p[4]]['union'].append(float(sl[6]))
    tmpf.close()
    
    for f in glob.glob('*.kch'):
        if os.path.isfile(f):
            os.remove(f)
        else:
            shutil.rmtree(f)

res = open(sys.argv[2], 'w')
for v in vres:
    res.write('Diff between versions : ' + v + '\n====================\n')
    #res.write('Added triples geometric mean : ' + str(gmean(vres[v]['added'])) + '\n')
    #res.write('Deleted triples geometric mean : ' + str(gmean(vres[v]['deleted'])) + '\n')
    #res.write('Number of triples geometric mean : ' + str(gmean(vres[v]['ntriples2'])) + '\n')
    #res.write('Percentage added geometric mean : ' + str(gmean(vres[v]['padds'])) + '\n')
    #res.write('Percentage deleted geometric mean : ' + str(gmean(vres[v]['pdels'])) + '\n')
    res.write('\n\nRaw results : \n')
    res.write('Added : ' + str(vres[v]['added']) + '\n')
    res.write('Deleted : ' + str(vres[v]['deleted']) + '\n')
    res.write('N triples file 1 : ' + str(vres[v]['ntriples1']) + '\n')
    res.write('N triples file 2 : ' + str(vres[v]['ntriples2']) + '\n')
    res.write('% adds : ' + str(vres[v]['padds']) + '\n')
    res.write('% dels : ' + str(vres[v]['pdels']) + '\n')
    res.write('Union : ' + str(vres[v]['union']) + '\n')
res.close()

param_file.close()
