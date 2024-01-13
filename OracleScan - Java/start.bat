@ECHO OFF
@SET CP=.;ojdbc6.jar

java -classpath %CP% OracleScan %*
