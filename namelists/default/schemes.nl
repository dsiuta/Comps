# Tag       Class Name           Scheme options

# DetMetric
normMetric  class=DetMetricNorm  order=1 weights=1

# Averager
mean        class=AveragerMean
median      class=AveragerMedian
weighted    class=AveragerWeighted useSelectorSkill

# Input
bogus       class=InputBogus     folder=bogus    type=forecast    mean=10 amplitude=5 period=24 members=15 ensSpread=13
bogusObs    class=InputBogus     folder=bogus    type=observation mean=12 amplitude=2 period=24 members=1
gfsSmall    class=InputGrib      folder=gfsSmall type=forecast maxCacheSize=8e9 allowTimeInterpolation  cacheOtherLocations  filenamePrefix=gfs_4_  filenameMiddle=_0000_
gfs         class=InputGrib      folder=gfs      type=forecast maxCacheSize=8e9 allowTimeInterpolation  cacheOtherLocations  filenamePrefix=gfs_4_  filenameMiddle=_0000_
gfsOp       class=InputNetcdf    folder=gfsOp    type=forecast maxCacheSize=8e9 allowTimeInterpolation  cacheOtherLocations  cacheOtherOffsets
rda336      class=InputRdaNetcdf folder=rda336   type=observation cacheOtherLocations  cacheOtherOffsets
lorenz63    class=InputLorenz63  folder=lorenz63 type=forecast        x0=0 y0=1.1 z0=0 dt=0.001 xVar=0.3 ensSize=6
lorenz63obs class=InputLorenz63  folder=lorenz63 type=observation     x0=0 y0=1 z0=0 dt=0.001 xVar=0.3
tutFcst     class=InputFlat      folder=tutFcst  type=forecast    useCodeInFilename fileExtension=txt
tutObs      class=InputFlat      folder=tutObs   type=observation useCodeInFilename fileExtension=txt

# Selectors
def         class=SelectorDefault
nomissing   class=SelectorDefault removeMissing
perf        class=SelectorPerformance num=3 detMetric=normMetric
an          class=SelectorAnalog      variables=T,RH,MSLP,WS analogMetric=normMetric numAnalogs=15 averager=mean normalize=1
clim        class=SelectorClim        dayLength=15 hourLength=0 allowWrappedOffsets allowFutureValues
pers        class=SelectorPersistence
pers0       class=SelectorPersistence useLatest

# Downscaler
nearest     class=DownscalerNearestNeighbour
interp      class=DownscalerInterpolation order=1 numPoints=16
downPerf    class=DownscalerPerformance   numPoints=16 detMetric=normMetric

# Corrector
poly        class=CorrectorPolynomialRegression useOrders=0,1
reg01       class=CorrectorPolynomialRegression useOrders=0,1
reg11       class=CorrectorPolynomialRegression useOrders=1,1
reg10       class=CorrectorPolynomialRegression useOrders=1,0 averager=mean
round0      class=CorrectorRound roundDownTo=0 memberSpecific
kf          class=CorrectorKalmanFilter ratio=0.1
Kmeans      class=CorrectorKmeans numMeans=3

# Continuous
mm2         class=ContinuousMoments  distribution=gaussian0 type=full efold=30 measure=ensVar
mm2x        class=ContinuousMoments  distribution=gaussian0 type=full efold=30 measure=ensMean
mm1         class=ContinuousMoments  distribution=gaussian0 type=ens efold=30 measure=ensVar
mm0         class=ContinuousMoments  distribution=gaussian0 type=const efold=30 measure=ensVar
gamma       class=ContinuousGamma    distribution=gamma efold=20 estimator=maxlikeilhood
bpe         class=ContinuousBpe      distribution=gaussian0 interp=linear

# Discrete
const       class=DiscreteConst     x=0                     efold=1000
consensus   class=DiscreteConsensus x=0
logit1      class=DiscreteLogit useConst                    efold=10 x=0
logit2      class=DiscreteLogit useConst  useMean           efold=10 x=0
logit3      class=DiscreteLogit useConst  useMean  useFrac  efold=10 x=0

# Measures
ensMean   class=MeasureEnsembleMoment moment=1
ensVar    class=MeasureEnsembleMoment moment=2 absolute

# Calibration
pitcal      class=CalibratorPitBased       numSmooth=9 efold=20 interp=linear

# Updater
pitupd      class=UpdaterPitBased

# BaseDistribution
gaussian0  class=BaseDistributionGaussian
gamma0     class=BaseDistributionGamma

# EstimatorProbabilistic
maxlikeilhood class=EstimatorMaximumLikelihood efold=30

# Smoother
triangle   class=SmootherTriangle width=5

# Output
oGfs       name=gfs      class=OutputNetcdf input=rda336
oLorenz    name=lorenz63 class=OutputNetcdf input=lorenz63
oTutorial  name=tutorial class=OutputNetcdf input=tutObs
oBogus     name=bogus    class=OutputNetcdf input=bogusObs

# Parameters Ios
parMemory  class=ParameterIoMemory finder=finder
parNetcdf  class=ParameterIoNetcdf finder=finder

# Finders
finder class=FinderDefault

# Interpolators
linear   class=InterpolatorLinear

# Metrics
mae      class=MetricMae
mse      class=MetricNorm order=2
bias     class=MetricBias
ign      class=MetricIgnorance
pit      class=MetricPit
sharp    class=MetricSharpness width=0.9
spread   class=MetricSpread
ensSpread class=MetricMeasure measure=ensVar
obs      class=MetricObs
fcst     class=MetricForecast
skill    class=MetricSelectorSkill type=min
th0      class=MetricThreshold threshold=0
th1      class=MetricThreshold threshold=1
th5      class=MetricThreshold threshold=5
p0       class=MetricThreshold threshold=0 useProbabilistic
p5       class=MetricThreshold threshold=5 useProbabilistic
p10      class=MetricThreshold threshold=10 useProbabilistic
p25      class=MetricThreshold threshold=25 useProbabilistic
crps     class=MetricCrps
brier5   class=MetricBrier threshold=5
brier6   class=MetricBrier threshold=60
brier10  class=MetricBrier threshold=10 anomalyAbove