gfs      inputsF=gfs inputsO=rda336 varconfs=T outputs=netcdf debug=10 writeForecasts  writeVerifications  getDist  qcs=qc locations=0:5 offsets=0:1:24 obsSelectors=obsOffset,obsNearest
tutorial inputsF=tutFcst inputsO=tutObs varconfs=T outputs=netcdf debug=50 writeForecasts  writeVerifications  getDist  parameterIo=parNetcdf obsSelectors=obsOffset,obsNearest

# Idealized runs
lorenz   inputsF=lorenz63 inputsO=lorenz63obs varconfs=X outputs=netcdf debug=50 writeForecasts  writeVerifications  getDist obsSelectors=obsOffset,obsNearest
sine     inputsF=sine inputsO=sineObs varconfs=T outputs=netcdf debug=50 writeForecasts  writeVerifications  getDist  parameterIo=parNetcdf qcs=qc obsSelectors=obsOffset,obsNearest

convert  inputsF=gfs   inputsO=rda336 varconfs=T outputs=netcdf debug=10 writeVerifications  metric=mae obsSelectors=obsOffset,obsNearest
