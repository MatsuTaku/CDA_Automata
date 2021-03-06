#!/bin/bash

dam() {
  ./daram/build/tools/daram_build $1 -o $2 >$2.stdout 2>&1
}
export -f dam

damcn() {
  ./daram/build/tools/daram_build $1 -o $2 --comp-next >$2.stdout 2>&1
}
export -f damcn

damci() {
  ./daram/build/tools/daram_build $1 -o $2 --comp-id >$2.stdout 2>&1
}
export -f damci

damcsi() {
  ./daram/build/tools/daram_build $1 -o $2 --comp-id --select-id >$2.stdout 2>&1
}
export -f damcsi

damdw() {
  ./daram/build/tools/daram_build $1 -o $2 --dac-cwords >$2.stdout 2>&1
}
export -f damdw

damac() {
  ./daram/build/tools/daram_build $1 -o $2 --access >$2.stdout 2>&1
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
    ./daram/build/tools/convert $orig $2 0 >> $2.stdout 2>&1
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
    ./daram/build/tools/convert $orig $2 1 >> $2.stdout 2>&1
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
  ./experiments/build/darts_build $1 $2 >$2.stdout 2>&1
}
export -f darts

samc() {
  ./experiments/build/samc_build $1 $2 >$2.stdout 2>&1
}
export -f samc


TOOLS="
dam
morfologik_fsa5d
morfologik_cfsa2d
darts
xcdat
fxcdat
marisa
centrp
"
#damdw
#damcn
#damci
#damcsi
#damac
#samc

DATASET_DIR=data-sets

DATASETS_LOCAL=`find $DATASET_DIR/local -name '*.dict' -not -name '*.1000000.dict'`
DATASETS_CIURA=`find $DATASET_DIR/ciura-deorowicz -name '*.dict' -not -name '*.1000000.dict'`
DATASETS_WEISS=`find $DATASET_DIR/weiss -name '*.dict' -not -name '*.1000000.dict'`
DATASETS="
$DATASETS_LOCAL 
"
#$DATASETS_WEISS
#$DATASETS_CIURA 

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

