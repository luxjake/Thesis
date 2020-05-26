#!/usr/bin/python3

import sys
import os
import subprocess
import time
import shutil

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
        pairs.append((line[0],line[1],line[2],line[3]))
        vres[line[3]] = {}
        vres[line[3]]['nt1'] = []
        vres[line[3]]['nt2'] = []
        vres[line[3]]['ent_ch'] = []
        vres[line[3]]['ent_add'] = []
        vres[line[3]]['ent_del'] = []
        vres[line[3]]['obj_up'] = []
        vres[line[3]]['obj_add'] = []
        vres[line[3]]['obj_del'] = []
        vres[line[3]]['comp_add'] = []
        vres[line[3]]['comp_del'] = []
        vres[line[3]]['ent_triples'] = []
        vres[line[3]]['total_comp'] = []

for p in pairs:
    if not os.path.isfile(p[0]):
        print('The file ' + p[0] + ' does not exist.')
        sys.exit()
    if not os.path.isfile(p[1]):
        print('The file ' + p[1] + ' does not exist.')
        sys.exit()
    
    print('Diff for files : ' + p[0] + ' & ' + p[1])
    subprocess.run(['memhighdiff', p[0], p[1], p[2]])
        
    tmpf = open(p[2] + '.res', 'r')
    l = tmpf.readline()
    sl = l.split(' ')
    vres[p[3]]['nt1'].append(float(sl[0]))
    vres[p[3]]['nt2'].append(float(sl[1]))
    vres[p[3]]['ent_ch'].append(float(sl[2]))
    vres[p[3]]['ent_add'].append(float(sl[3]))
    vres[p[3]]['ent_del'].append(float(sl[4]))
    vres[p[3]]['obj_up'].append(float(sl[5]))
    vres[p[3]]['obj_add'].append(float(sl[6]))
    vres[p[3]]['obj_del'].append(float(sl[7]))
    vres[p[3]]['comp_add'].append(float(sl[8]))
    vres[p[3]]['comp_del'].append(float(sl[9]))
    vres[p[3]]['ent_triples'].append(float(sl[10]))
    vres[p[3]]['total_comp'].append(float(sl[11]))
    tmpf.close()
    
    shutil.rmtree('./database/')

res = open(sys.argv[2], 'w')
for v in vres:
    res.write('High level diff between versions : ' + v + '\n====================\n')
    res.write('\n\nRaw results : \n')
    res.write('Entity changes : ' + str(vres[v]['ent_ch']) + '\n')
    res.write('Entity additions : ' + str(vres[v]['ent_add']) + '\n')
    res.write('Entity deletions : ' + str(vres[v]['ent_del']) + '\n')
    res.write('Entity changes triples : ' + str(vres[v]['ent_triples']) + '\n')
    res.write('Object updates : ' + str(vres[v]['obj_up']) + '\n')
    res.write('Object additions : ' + str(vres[v]['obj_add']) + '\n')
    res.write('Object deletions : ' + str(vres[v]['obj_del']) + '\n')
    res.write('Total components : ' + str(vres[v]['total_comp']) + '\n')
    res.write('Components additions : ' + str(vres[v]['comp_add']) + '\n')
    res.write('Components deletions : ' + str(vres[v]['comp_del']) + '\n')
    res.write('N triples file 1 : ' + str(vres[v]['nt1']) + '\n')
    res.write('N triples file 2 : ' + str(vres[v]['nt2']) + '\n')
    res.write('\n')
res.close()

param_file.close()
