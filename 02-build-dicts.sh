#!/bin/bash

dam() {
  ./build/dam_build $1 $2 >$2.stdout 2>&1
}
export -f dam

damac() {
  ./build/dam_build $1 $2 --access >$2.stdout 2>&1
}
export -f damac

morfologik_fsa5() {
  java -server -Xms50m -jar software/cfsa2.nx/morfologik*.jar \
    fsa_build \
       --format fsa5 \
       --progress \
       --sorted -i $1 \
       -o $2 >$2.stdout 2>&1
  ./build/convert $2 $2 0 >> $2.stdout 2>&1
}
export -f morfologik_fsa5

morfologik_cfsa2() {
  java -server -Xms50m -jar software/cfsa2.nx/morfologik*.jar \
    fsa_build \
       --format cfsa2 \
       --progress \
       --sorted -i $1 \
       -o $2 >$2.stdout 2>&1
  ./build/convert $2 $2 1 >> $2.stdout 2>&1
}
export -f morfologik_cfsa2

xcdat() {
  ./software/xcdat/xcdat build 1 $1 $2 >$2.stdout 2>&1
}
export -f xcdat

centrp() {
  ./software/path_decomposed_tries/tries_perftest centroid_repair prepare $1 $2 >$2.stdout 2>&1
}
export -f centrp

marisa() {
  marisa-build -o $2 $1 >$2.stdout 2>&1
}
export -f marisa

darts() {
  ./build/darts_build $1 $2 >$2.stdout 2>&1
}
export -f darts


TOOLS="
damac
dam
morfologik_fsa5
morfologik_cfsa2
darts
xcdat
marisa
centrp
"

DATASET_DIR=data-sets

DATASETS=`find $DATASET_DIR -name '*.dict' -not -name '*.1000000.dict'`
echo '------Build list------'
for f in $DATASETS ; do
	echo $f
done
echo '------'

RESULTS_DIR=results
export RESULTS_DIR

TIME_CMD=gtime
export TIME_CMD
TIME_FMT="wall\t%e\t%E\tuser\t%U\tsys\t%S\tmax-mem-kb\t%M\tdta\t%D\tinputs\t%I\toutputs\t%O"
export TIME_FMT

build() {
	echo "$1 <- $2..."
	dataset_fn=`basename $2 .dict`
	mkdir -p $RESULTS_DIR/$dataset_fn
	outname=$RESULTS_DIR/$dataset_fn/$dataset_fn.$1
	$TIME_CMD -f $TIME_FMT -o $outname.log bash -c "$1 $2 $outname"
}
export -f build

cd $(dirname $0)

THREADS=1
if [ $# -ge 1 ] ; then
	THREADS=$1
fi

parallel -j $THREADS build {1} {2} ::: $TOOLS ::: $DATASETS 

