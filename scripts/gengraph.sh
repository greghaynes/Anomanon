#!/bin/sh

usage ()
{
	echo "$0 rrd_db out_image start_time source_vairable"
}

if [ -z $1 ]
then
	usage
	exit
fi

if [ -z $2 ]
then
	usage
	exit
fi

if [ -z $3 ]
then
	usage
	exit
fi

if [ -z $4 ]
then
	usage
	exit
fi

rrdtool graph $2 --start $3\
	DEF:obs=$1:$4:AVERAGE \
	DEF:pred=$1:$4:HWPREDICT \
	DEF:dev=$1:$4:DEVPREDICT \
	DEF:fail=$1:$4:FAILURES \
	CDEF:upper=pred,dev,2,*,+ \
	CDEF:lower=pred,dev,2,*,- \
	TICK:fail#ffffa0:1.0:"Failures\: Average bytes out" \
	LINE2:obs#0000ff:"Average bits out" \
	LINE1:upper#ff0000:"Upper Confidence Bound\: Average bytes out" \
	LINE1:lower#ff0000:"Lower Confidence Bound\: Average bytes out"
	
