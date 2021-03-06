import matplotlib.pyplot as mpl
import numpy as np
from matplotlib.dates import *
import os
from matplotlib.dates import YearLocator, MonthLocator, DateFormatter, DayLocator, HourLocator, WeekdayLocator
class Plot:
   def __init__(self, file):
      self.file = file
      self._setDefaults()
      self.showSkill = False
      self.dst = False

   def setDst(self, flag):
      self.dst = flag

   def _setDefaults(self):
      self.ms    = 4.0  # Marker size
      self.lw    = 1
      self.green = [0,1,0]
      self.blue  = [0,0,1]
      self.red   = [1,0,0]
      self.imgRes = 100
      self.showX = 1
      self.showTitle = 1
      self.fs = 10
      self.labelFs = 10
      self.showGrid = 1
      self.minOffset = np.nan;
      self.maxOffset = np.nan;

   def setOffsets(self, offsets):
      self.offsets = offsets

   def plot(self, ax):
      self.plotCore(ax)
      if(self.showGrid):
         mpl.grid('on')
      else:
         mpl.grid('off')

   def plotCore(self, ax):
      assert False, "Not implemented"

   def labelAxes(self):
      mpl.xlabel("Time (PDT)", fontsize=self.labelFs, position=[0.5,0.1])

   def disableX(self):
      self.showX = 0

   def disableTitle(self):
      self.showTitle = 0;

   # When set to true, will colour the ensemble dots based on how skillful they are predicted to be.
   # Useful for analogs
   def setShowSkill(self, b):
      self.showSkill = b;

   def setFontSize(self, fs):
      self.fs = fs

   # Fill an area along x, between yLower and yUpper
   # Both yLower and yUpper most correspond to points in x (i.e. be in the same order)
   def _fill(self, x, yLower, yUpper, col, alpha=1, zorder=0):
      # This approach doesn't work, because it doesn't remove points with missing x or y
      #X = np.hstack((x, x[::-1]))
      #Y = np.hstack((yLower, yUpper[::-1]))

      # Populate a list of non-missing points
      X = list()
      Y = list()
      for i in range(0,len(x)):
         if(not( np.isnan(x[i]) or np.isnan(yLower[i]))):
            X.append(x[i])
            Y.append(yLower[i])
      for i in range(len(x)-1, -1, -1):
         if(not (np.isnan(x[i]) or np.isnan(yUpper[i]))):
            X.append(x[i])
            Y.append(yUpper[i])
      mpl.fill(X, Y, facecolor=col, alpha=alpha,linewidth=0, zorder=zorder)

# Generic (abstract) plot with time as x-axis
class TimePlot(Plot):
   def __init__(self, file):
      Plot.__init__(self, file)
      self.shortRange = True

   def setShortRange(self, flag):
       self.shortRange = flag

   def _xAxis(self, ax):
      # X-axis labels
      # Don't create ticks when the x-axis range is too big. Likely this is because of
      # a problem with the input data. Some versions of python crash when trying to 
      # create too many ticks
      range = mpl.xlim()
      if(range[1] - range[0] < 100):
         if(self.shortRange):
            mpl.gca().xaxis.set_major_locator(DayLocator(interval=1))
            mpl.gca().xaxis.set_minor_locator(HourLocator(interval=6))
            mpl.gca().xaxis.set_major_formatter(DateFormatter('\n  %a %d %b %Y'))
            mpl.gca().xaxis.set_minor_formatter(DateFormatter('%H'))
         else:
            mpl.gca().xaxis.set_major_locator(WeekdayLocator(byweekday=(MO,TU,WE,TH,FR)))
            mpl.gca().xaxis.set_major_formatter(DateFormatter('\n%Y-%m-%d'))
            mpl.gca().xaxis.set_minor_locator(WeekdayLocator(byweekday=(SA,SU)))
            mpl.gca().xaxis.set_minor_formatter(DateFormatter('\n%Y-%m-%d'))
            mpl.xticks(rotation=90)


      if(self.showX):
         mpl.xlabel('Date', fontsize=self.labelFs)

      majlabels = [tick.label1 for tick in mpl.gca().xaxis.get_major_ticks()]
      for i in majlabels:
         # Don't show the last label, since it will be outside the range
         if(i == majlabels[len(majlabels)-1]):
            i.set_visible(0)
         if(not self.showX):
            i.set_visible(0);
         else:
            if(self.shortRange):
               i.set_horizontalalignment('left')
               i.set_position((0,-0.035))
            else:
               i.set_horizontalalignment('right')
               i.set_rotation(30);
            i.set_verticalalignment('top')
            i.set_fontsize(self.fs)
            i.set_position((0,-0.035))
      minlabels = [tick.label1 for tick in mpl.gca().xaxis.get_minor_ticks()]
      for i in minlabels:
         if(not self.showX):
            i.set_visible(0);
         else:
            if(self.shortRange):
               i.set_horizontalalignment('center')
               i.set_rotation(0);
               i.set_color("k")
            else:
               i.set_horizontalalignment('right')
               i.set_rotation(30);
               i.set_color((1,0,1))       # Weekend days are magenta
            i.set_verticalalignment('top')
            i.set_fontsize(self.fs)

      ylabels = [tick.label1 for tick in mpl.gca().yaxis.get_major_ticks()]
      for i in ylabels:
         i.set_fontsize(self.fs)

      # Gridlines
      mpl.gca().xaxis.grid(True, which='major', color='k', zorder=-10, linestyle='-')
      if(self.shortRange):
         mpl.gca().xaxis.grid(True, which='minor', color='k', zorder=0, linestyle=':')
      else:
         mpl.gca().xaxis.grid(True, which='minor', color=(1,0,1), zorder=0, linestyle='-')

      minOffset = min(self.file.getOffsets())

      maxOffset = max(self.file.getOffsets())
      if(not np.isnan(self.maxOffset)):
         maxOffset = minOffset + self.maxOffset/24.0
      mpl.xlim(minOffset, maxOffset)

   def setOffsetRange(self, min, max):
      self.minOffset = min;
      self.maxOffset = max;

   def getMarkerSize(self, i):
      if(not self.showSkill):
         mss = 6
      elif(i == 0):
         mss = self.ms
      elif(i == 1):
         mss = self.ms*28/36
      elif(i == 2):
         mss = self.ms*16/36
      else:
         mss = self.ms*10/36
      return mss

   def getMarkerColor(self, i):
      if(not self.showSkill):
         col = 'k'
      elif(i == 0):
         col = [0,0,0.6]
      elif(i == 1):
         col = [0.3,0.3,1]
      elif(i == 2):
         col = [0.7,0.7,1]
      else:
         col = self.red
      return col


   def getMarkerEdgeColor(self, i):
      if(not self.showSkill):
         mec = 'w'
      elif(i < 3):
         mec = 'k'
      else:
         mec = self.getMarkerColor(i)
      return mec

   def getMarkerStyle(self, i):
      if(not self.showSkill):
         mstyle = '.'
      else:
         mstyle = 'o'
      return mstyle


###########################
# Single click meteograms #
###########################
class MeteoPlot(TimePlot):
   def __init__(self, file):
      Plot.__init__(self, file)
      # Default colours (not used, always overridden)
      self.col = [0,0,0]
      self.shading = [0.1,0.1,0.1]
      
      # Opaqueness of the shading
      self.alpha = 0.3

      # Set the size and position of the axis in the figure
      mpl.gca().get_axes().set_position([0.1, 0.2, 0.87, 0.75])

      self.setStyle("ubc")

   # Set the style of the plots:
   #    cmbc:
   def setStyle(self, style):
      if(not (style == "ubc" or style == "cmbc")):
         error('test')

      self.style = style
      if(style == "ubc"):
         self.gridStyle = '--';
      elif(style == "cmbc"):
         self.gridStyle = '-';

   # Set colour of lines and shadings for current plot
   def setCol(self, col):
      self.col = col;
      shade = 0.6
      # Shading should be: [1 shade shade] or [shade shade 1]
      self.shading = [(col[0]>0)*(1-shade)+shade, (col[1]>0)*(1-shade)+shade, (col[2]>0)*(1-shade)+shade]

   def plotCore(self,ax):
      ens = self.file.getEnsemble()
      obs = self.file.getObs()
      dets = self.file.getDeterministic()
      # Plots mean
      mpl.plot(ens['offsets'], dets['values'], '-', color=self.col);

      # Plot shading
      self._fill(ens['offsets'], ens['values'][:,0], ens['values'][:,2], self.shading,
            self.alpha, zorder=-20)

      # Plot obs
      mpl.plot(obs['offsets'], obs['values'],'.', color=self.col);

      var = self.file.getVariable()
      mpl.ylabel(var['name'] + " (" + var['units'] + ")", fontsize=self.labelFs)
      self._xAxis(ax)
      self._yAxis(ax)

   def _yAxis(self, ax):
      if(self.style == "cmbc"):
         #mpl.gca().yaxis.set_major_locator(MultipleLocator(2))

         [y_start, y_end] = mpl.ylim();
         '''
         y_start = min(ylims[:,0]);
         y_end   = max(ylims[:,1]);
         # Determine ylimits
         if(y_start == -999):
            y_start = -20
         else:
            y_start = np.floor(y_start/2)*2
         if(y_end == -999):
            y_end = 10
         else:
            y_end = np.ceil(y_end/2)*2

         # Always show at least down to -5
         if(y_start > -5):
            y_start = -5;
         mpl.ylim([y_start, y_end]);
         '''

      [y_start, y_end] = mpl.ylim();

   # Format x-axis
   def _xAxis(self, ax):
      # Set range
      mpl.xlim(np.floor(min(self.file.getOffsets())), np.floor(max(self.file.getOffsets())))

      # X-axis labels
      mpl.gca().xaxis.set_major_locator(DayLocator(interval=1))
      mpl.gca().xaxis.set_minor_locator(HourLocator(interval=6))
      mpl.gca().xaxis.set_major_formatter(DateFormatter('\n  %a %d %b %Y'))
      mpl.gca().xaxis.set_minor_formatter(DateFormatter('%H'))

      # Hour labels
      minlabels = [tick.label1 for tick in mpl.gca().xaxis.get_minor_ticks()]
      for i in minlabels:
         i.set_fontsize(12)

      # Date labels
      majlabels = [tick.label1 for tick in mpl.gca().xaxis.get_major_ticks()]
      counter = 0
      numLabels = 4;
      for i in majlabels:
         if(counter < numLabels):
            i.set_horizontalalignment('left')
            i.set_verticalalignment('top')
            i.set_fontsize(12)
            i.set_position((0,-0.035))    # Moves major labels to the top of the graph
                                          # The x-coordinate seems to be irrelevant. When y-coord
                                          # is 1, the label is near the top. For 1.1 it is above the graph
         else:
            i.set_visible(0)              # Turn off the last date label, since it is outside the graph
         counter = counter + 1

      mpl.gca().xaxis.grid(True, which='major', color='k', zorder=-10, linestyle='-', linewidth=2)
      mpl.gca().xaxis.grid(True, which='minor', color='k', zorder=0, linestyle=self.gridStyle)
      mpl.gca().yaxis.grid(True, which='major', color='k', zorder=0)
      if(self.dst):
         tzLabel = "PDT"
      else:
         tzLabel = "PST"
      mpl.xlabel("Past                Time (" + tzLabel + ")             Future", fontsize=15, position=[0.5, 0.1])

      mpl.gcf().set_size_inches(12,4)

class CdfPlot(TimePlot):

   def __init__(self, file):
      TimePlot.__init__(self, file)
      self._showEns = True
      self._showProb = True
      self._showObs = True

   def setShowEns(self, flag):
      self._showEns = flag

   def setShowProb(self, flag):
      self._showProb = flag

   def setShowObs(self, flag):
      self._showObs = flag

   def plotCore(self, ax):
      ens = self.file.getEnsemble()
      self._plotObs(ax)
      self._plotDeterministic(ax)
      if(self._showEns):
         self._plotEnsemble(ax)
      if(self._showProb):
         self._plotProb(ax)
      var = self.file.getVariable()
      mpl.ylabel(var['name'] + " (" + var['units'] + ")", fontsize=self.labelFs)
      self._xAxis(ax)
      if(self.showTitle):
         loc = self.file.getLocation()
         mpl.title('Meteogram for ' + "%d %2.2f %2.2f" % (loc['id'],loc['lat'], loc['lon']), fontsize=self.fs);

   def _plotObs(self, ax):
      if(self._showObs):
         obs = self.file.getObs()
         mpl.plot(obs['offsets'], obs['values'], 'o-', mfc='w', mew=2, color=self.red,
               mec=self.red, ms=self.ms*3/4, lw=self.lw, label="Obs", zorder=5)

   # Draw one dot for each ensemble member
   def _plotEnsemble(self, ax):
      ens = self.file.getEnsemble()
      nMembers =  ens['values'].shape[1]
      for i in range(0,nMembers):
         col = self.getMarkerColor(i)
         mss = self.getMarkerSize(i)
         mec = self.getMarkerEdgeColor(i)
         mstyle = self.getMarkerStyle(i)

         if(i == 0):
            mpl.plot(ens['offsets'], ens['values'][:,i], mstyle, mec=mec, ms=mss, mfc=col,
                  label="Ens members");
         else:
            mpl.plot(ens['offsets'], ens['values'][:,i], mstyle, mec=mec, ms=mss, mfc=col);

   def plotMember(self, ax, member, col, name=""):
      ens = self.file.getEnsemble()
      mss = 5
      mec = 'k'
      mpl.plot(ens['offsets'], ens['values'][:,member], '-s', lw=2, mfc=col, color=col, mec=mec, ms=mss, label=name);

   # Plots CDF lines
   def _plotProb(self, ax):
      cdf = self.file.getCdfs()
      nLines =  cdf['values'].shape[1]
      for i in range(nLines-1,-1,-1):
         if(i < (nLines-1)/2.0):
            var = 1-float(i)/((nLines-1)/2);
            ec  = [0,0,var]; # Edgecolour
            faceCol = var
         else:
            var = (i - (nLines-1)/2.0)/(float(nLines-1)/2+1)
            ec = [var,0,0];
            faceCol = (i+1- (nLines-1)/2.0)/(float(nLines-1)/2)
         if(i == (nLines-1)/2.0):
            ec = [0,1,0]

         col = [faceCol,faceCol,faceCol];
         if(i == 0 or i == nLines-1):
            mstyle = '--'
         else:
            mstyle = '-'
         lbl = "%d" % (round(cdf['cdfs'][i]*100.0)) + "%"
         mpl.plot(cdf['offsets'], cdf['values'][:,i], mstyle, color=ec, lw=self.lw,
               label=lbl, zorder=-10);
         if(i < nLines-1):
            # Only plot if not all values are missing
            if(sum(np.isnan(cdf['values'][:,i])) < len(cdf['values'][:,0])):
               self._fill(cdf['offsets'], cdf['values'][:,i], cdf['values'][:,i+1], col,
                     zorder=-20)

   def _plotDeterministic(self, ax):
      dets = self.file.getDeterministic()
      mpl.plot(dets['offsets'], dets['values'], 'o-', mfc=[1,1,1], mew=2,
            color=self.green, mec=self.green, ms=self.ms*3/4, lw=self.lw,
            label="Deterministic");

class DiscretePlot(TimePlot):

   def __init__(self, file):
      Plot.__init__(self, file)
      self.invertY = 0;

   def setInvertY(self, flag):
      self.invertY = flag

   def plotCore(self, ax):
      self._plotProb(ax)
      var = self.file.getVariable()
      if(var['name'] == "Precip24"):
         ylab = "Prob of Precip (%)"
      else:
         ylab = "Probability (%)"
      mpl.ylabel(ylab, fontsize=self.labelFs)

      self._xAxis(ax)
      mpl.ylim([0,100]);
      if(self.showTitle):
         mpl.title('Meteogram for ' + str(self.file.getLocation()['id']), fontsize=self.fs);

   # Plots CDF lines
   def _plotProb(self, ax):
      p0 = self.file.getLowerDiscrete()
      y = p0['values'][:]
      if(self.invertY):
         y = 1 - y;
      mpl.plot(p0['offsets'], 100*y, 'k-', mew=2);

# Shows which dates were used to construct ensemble. Useful for analogs.
class DatesPlot(TimePlot):
   def plotCore(self, ax):
      dates = self.file.getDates()
      nMembers =  dates['values'].shape[1]

      # Only draw if there are valid points.
      # mpl crashes otherwise
      if(dates['values'].size - np.isnan(dates['values']).sum() > 0):
         mpl.gca().yaxis.set_major_locator(YearLocator())
         mpl.gca().yaxis.set_major_formatter(DateFormatter('\n%b/%Y'))
         mpl.gca().yaxis.set_minor_locator(MonthLocator(interval=1))
         for i in range(0, nMembers):
            col = self.getMarkerColor(i)
            mss = self.getMarkerSize(i)
            mec = self.getMarkerEdgeColor(i)
            mstyle = self.getMarkerStyle(i)
            mpl.plot(dates['offsets'], dates['values'][:,i], mstyle, mec=mec, ms=mss, mfc=col);
      self._xAxis(ax)
      mpl.title('Dates used to construct ensembles', fontsize=self.fs)
