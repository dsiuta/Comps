import matplotlib
matplotlib.use('Agg')
import os
import numpy as np
from copy import deepcopy
from matplotlib.dates import YearLocator, MonthLocator, DateFormatter
from scipy.io import netcdf
from pylab import *
import sys
import scipy

def cleanData(data):
   data = data[:].astype(float)
   q = deepcopy(data)
   mask = where(q == -999);
   q[mask] = None
   mask = where(q < -100000);
   q[mask] = None
   mask = where(q > 1e30);
   q[mask] = None
   return q

f = netcdf.netcdf_file(sys.argv[1], 'r')
var = sys.argv[2];
if(len(sys.argv) == 4):
   tperiod = int(sys.argv[3])
   print "Training period: " + str(tperiod) + " day"
else:
   tperiod = 0

data = cleanData(f.variables[var])
data = data[range(tperiod,size(data,0)),:,:]
mdat = np.ma.masked_array(data,np.isnan(data))
if(var == "pit"):
   N = 10;
   edges = np.linspace(0,1,N+1);
   centers = (edges[range(0,len(edges)-1)] + edges[range(1,len(edges))])/2
   widths =  edges[range(1,len(edges))] - edges[range(0,len(edges)-1)]
   d = zeros(size(mdat,1),'float');
   # Loop over each offst
   for i in range(0, size(mdat,1)):
      [nums,qq] = np.histogram(squeeze(mdat[:,i,:]), edges)
      # Expected amount
      e = sqrt((1 - 1/float(N))/(float(sum(nums))*N))
      print e
      if(sum(nums) > 0):
         freq = nums / float(sum(nums))
         d[i] = sqrt(mean(abs(freq-widths)**2))/e*100 - 100
      else:
         d[i] = None
   m = mean(d)
else:
   d = squeeze(np.mean(np.mean(mdat,axis=2),axis=0))
   m = mean(d)
s = ''
for i in range(len(d)):
   s += '%2.2f ' %  d[i]

print 'Num dates: %d' % len(data[:,0])
print 'Num locations: %d' % len(data[0,0,:])
if(var == "pit"):
   print "% extra calibrated than expected perfect"
print 'Means: ' + s
print 'Mean: %2.2f' % m

#plot(d)
#show()