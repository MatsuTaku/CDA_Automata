#!/bin/bash

function dam {
  ./build/dam_bench $1 $2 11 >$1.bench.stdout 2>&1
}
export -f dam

function damac {
  ./build/dam_bench $1 $2 2 >$1.bench.stdout 2>&1
}
export -f damac

function morfologik_fsa5 {
  ./build/dam_bench $1 $2 9 >$1.bench.stdout 2>&1
}
export -f morfologik_fsa5

function morfologik_cfsa2 {
  ./build/dam_bench $1 $2 10 >$1.bench.stdout 2>&1
}
export -f morfologik_cfsa2

function xcdat {
  ./build/dam_bench $1 $2 7 >$1.bench.stdout 2>&1
}
export -f xcdat

function marisa {
  ./build/dam_bench $1 $2 8 >$1.bench.stdout 2>&1
}
export -f marisa

function darts {
  ./build/dam_bench $1 $2 12 >$1.bench.stdout 2>&1
}
export -f darts

function centrp {
  ./software/path_decomposed_tries/tries_perftest centroid_repair measure $1 $2 >$1.bench.stdout 2>&1
  cat $1.stdout >> $1.bench.stdout
}
export -f centrp


TOOLS="
damac
dam
morfologik_fsa5
morfologik_cfsa2
xcdat
centrp
marisa
darts
"

DATASET_DIR=data-sets

DATASETS="
$DATASET_DIR/kanda/enwiki-20150205.1000000.rnd_dict
$DATASET_DIR/kanda/jawiki-20150118.1000000.rnd_dict
$DATASET_DIR/kanda/indochina-2004.1000000.rnd_dict
$DATASET_DIR/kanda/word-over999.1000000.rnd_dict
$DATASET_DIR/weiss/pl.1000000.rnd_dict
$DATASET_DIR/weiss/wikipedia.1000000.rnd_dict
$DATASET_DIR/weiss/wikipedia2.1000000.rnd_dict
$DATASET_DIR/ciura-deorowicz/abc.1000000.rnd_dict
$DATASET_DIR/ciura-deorowicz/deutsch.1000000.rnd_dict
$DATASET_DIR/ciura-deorowicz/dimacs.1000000.rnd_dict
$DATASET_DIR/ciura-deorowicz/enable.1000000.rnd_dict
$DATASET_DIR/ciura-deorowicz/english.1000000.rnd_dict
$DATASET_DIR/ciura-deorowicz/eo.1000000.rnd_dict
$DATASET_DIR/ciura-deorowicz/esp.1000000.rnd_dict
$DATASET_DIR/ciura-deorowicz/files.1000000.rnd_dict
$DATASET_DIR/ciura-deorowicz/fr.1000000.rnd_dict
$DATASET_DIR/ciura-deorowicz/full.1000000.rnd_dict
$DATASET_DIR/ciura-deorowicz/ifiles.1000000.rnd_dict
$DATASET_DIR/ciura-deorowicz/one.1000000.rnd_dict
$DATASET_DIR/ciura-deorowicz/polish.1000000.rnd_dict
$DATASET_DIR/ciura-deorowicz/random.1000000.rnd_dict
$DATASET_DIR/ciura-deorowicz/russian.1000000.rnd_dict
$DATASET_DIR/ciura-deorowicz/sample.1000000.rnd_dict
$DATASET_DIR/ciura-deorowicz/scrable.1000000.rnd_dict
$DATASET_DIR/ciura-deorowicz/test.1000000.rnd_dict
$DATASET_DIR/ciura-deorowicz/unix_m.1000000.rnd_dict
$DATASET_DIR/ciura-deorowicz/unix.1000000.rnd_dict
$DATASET_DIR/ciura-deorowicz/webster.1000000.rnd_dict
"

RESULTS_DIR=results
export RESULTS_DIR

bench() {
    dataset_fn=`basename $2 .1000000.rnd_dict`
	dictname=$RESULTS_DIR/$dataset_fn/$dataset_fn.$1
	echo "benchmark [$1] $dictname..."
	$1 $dictname $2
}
export -f bench


python $DATASET_DIR/create-test-datasets.py

parallel -j 2 bench {1} {2} ::: $TOOLS ::: $DATASETS

