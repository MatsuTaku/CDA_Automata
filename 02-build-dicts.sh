#!/bin/bash

dam() {
  ./build/dam_build $1 -o $2 >$2.stdout 2>&1
}
export -f dam

damcn() {
  ./build/dam_build $1 -o $2 --comp-next >$2.stdout 2>&1
}
export -f damcn

damci() {
  ./build/dam_build $1 -o $2 --comp-id >$2.stdout 2>&1
}
export -f damci

damac() {
  ./build/dam_build $1 -o $2 --access >$2.stdout 2>&1
}
export -f damac

morfologik_fsa5() {
  java -server -Xms50m -jar software/cfsa2.nx/morfologik*.jar \
    fsa_build \
       --format fsa5 \
       --progress \
       --sorted -i $1 \
       -o $2 >$2.stdout 2>&1
}
export -f morfologik_fsa5

morfologik_fsa5d() {
    orig=${2%d}
    morfologik_fsa5 $1 $orig
    ./build/convert $orig $2 0 >> $2.stdout 2>&1
}
export -f morfologik_fsa5d

morfologik_cfsa2() {
  java -server -Xms50m -jar software/cfsa2.nx/morfologik*.jar \
    fsa_build \
       --format cfsa2 \
       --progress \
       --sorted -i $1 \
       -o $2 >$2.stdout 2>&1
}
export -f morfologik_cfsa2

morfologik_cfsa2d() {
    orig=${2%d}
    morfologik_cfsa2 $1 $orig
    ./build/convert $orig $2 1 >> $2.stdout 2>&1
}
export -f morfologik_cfsa2d

xcdat() {
  ./software/xcdat/xcdat build 1 $1 $2 >$2.stdout 2>&1
}
export -f xcdat

fxcdat() {
  ./software/xcdat/xcdat build 2 $1 $2 >$2.stdout 2>&1
}
export -f fxcdat

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
dam
damcn
damci
morfologik_fsa5d
morfologik_cfsa2d
darts
xcdat
fxcdat
marisa
centrp
"
#damac

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
echo 'Threads: '$THREADS
parallel -j $THREADS build {1} {2} ::: $TOOLS ::: $DATASETS 

