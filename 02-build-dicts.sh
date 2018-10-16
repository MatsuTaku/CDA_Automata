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

DATASETS="
$DATASET_DIR/kanda/enwiki-20150205.dict
$DATASET_DIR/kanda/jawiki-20150118.dict
$DATASET_DIR/kanda/indochina-2004.dict
$DATASET_DIR/kanda/word-over999.dict
$DATASET_DIR/weiss/pl.dict
$DATASET_DIR/weiss/wikipedia.dict
$DATASET_DIR/weiss/wikipedia2.dict
$DATASET_DIR/ciura-deorowicz/abc.dict
$DATASET_DIR/ciura-deorowicz/deutsch.dict
$DATASET_DIR/ciura-deorowicz/dimacs.dict
$DATASET_DIR/ciura-deorowicz/enable.dict
$DATASET_DIR/ciura-deorowicz/english.dict
$DATASET_DIR/ciura-deorowicz/eo.dict
$DATASET_DIR/ciura-deorowicz/esp.dict
$DATASET_DIR/ciura-deorowicz/files.dict
$DATASET_DIR/ciura-deorowicz/fr.dict
$DATASET_DIR/ciura-deorowicz/full.dict
$DATASET_DIR/ciura-deorowicz/ifiles.dict
$DATASET_DIR/ciura-deorowicz/one.dict
$DATASET_DIR/ciura-deorowicz/polish.dict
$DATASET_DIR/ciura-deorowicz/random.dict
$DATASET_DIR/ciura-deorowicz/russian.dict
$DATASET_DIR/ciura-deorowicz/sample.dict
$DATASET_DIR/ciura-deorowicz/scrable.dict
$DATASET_DIR/ciura-deorowicz/test.dict
$DATASET_DIR/ciura-deorowicz/unix_m.dict
$DATASET_DIR/ciura-deorowicz/unix.dict
$DATASET_DIR/ciura-deorowicz/webster.dict
"

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

parallel -j 4 build {1} {2} ::: $TOOLS ::: $DATASETS 

