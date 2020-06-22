#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import os.path
import numpy as np
import pylab as pl
from matplotlib import collections  as mc
from matplotlib.patches import Arc
import matplotlib.patches as mpatches
import math
import matplotlib._color_data as mcd
from shapely.geometry import *

def read_test(path):
    try:
        with open(path, 'r') as f:
            lines = f.readlines()
    except:
        print ('FATAL ERROR: can not read file {}, will skip it'.format(path))
        sys.exit(-1)

    data = []

    for line in lines:
        d = line.strip().split(' ')
        d = list(filter(None, d))
        #print (d)
        data.append(d)

    res = []

    try:
        for dl in data:
            if dl[0] == 'border':
                res.append([])
            if dl[0] == 'hole' or dl[0] == 'border':
                res[-1].append([])
            elif dl[0] == 'segment':
                res[-1][-1].append( ('segment', [(float(dl[1]), float(dl[2])), (float(dl[3]), float(dl[4])) ]))
            elif dl[0] == 'arc':
                res[-1][-1].append( ('arc', [float(dl[1]), float(dl[2]), float(dl[3]), math.degrees(float(dl[4])), math.degrees(float(dl[5])) ]))
            elif dl[0] == 'arc_degrees':
                res[-1][-1].append( ('arc', [float(dl[1]), float(dl[2]), float(dl[3]), float(dl[4]), float(dl[5]) ]))

    except Exception as e:
        print (e)
        print('FATAL ERROR: please check file {}, will skip it'.format(path))
        sys.exit(-1)

    return res

if __name__ == '__main__':
    fig, ax = pl.subplots()

    handles = []
    colors = [name for name in mcd.XKCD_COLORS ]
    for i, f in enumerate(sys.argv[1:]):
        #print (f)

        color = colors[ i % len(colors) ]
        polygons = read_test(f)

        add_legend_patch = False

        for polygon in polygons:
            for contour in polygon:
                segments = []
                for edge in contour:
                    if edge[0] == 'arc':
                        arc = edge[1]
                        ax.add_patch(Arc((arc[0], arc[1]), 2 * arc[2], 2 * arc[2], edgecolor=color, theta1=arc[3], lw=3, theta2=arc[4]))
                        add_legend_patch = True
                    elif edge[0] == 'segment':
                        segment = edge[1]
                        segments.append(segment)
                        add_legend_patch = True

                lc = mc.LineCollection(segments, linewidths=3, color = color)
                ax.add_collection(lc)

        if add_legend_patch:
            handles.append(mpatches.Patch(color=color, label=f))

    pl.axis('equal')
    pl.legend(handles=handles)
    
    pl.show()
