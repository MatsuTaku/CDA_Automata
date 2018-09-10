#!/bin/bash

#function morfologik_fsa5 {
#  ./src/build/bench $1 $2 1 >$1.bench.stdout 2>&1
#}
#
#function morfologik_cfsa2 {
#  ./src/build/bench $1 $2 2 >$1.bench.stdout 2>&1
#}

function xcdat {
  ./src/build/bench $1 $2 7 >$1.bench.stdout 2>&1
}

function centrp {
  ./software/path_decomposed_tries/perftest/tries_perftest centroid_repair measure $1 $2 >$1.bench.stdout 2>&1
}

function marisa {
  ./src/build/bench $1 $2 9 >$1.bench.stdout 2>&1
}

function array_fsa {
  ./src/build/bench $1 $2 0 >$1.bench.stdout 2>&1
}

function dac_array_fsa {
  ./src/build/bench $1 $2 1 >$1.bench.stdout 2>&1
}

function array_ts_fsa {
  ./src/build/bench $1 $2 2 >$1.bench.stdout 2>&1
}

function array_ts_fsa_noCuWo {
  ./src/build/bench $1 $2 3 >$1.bench.stdout 2>&1
}

function array_ts_fsa_noBro {
  ./src/build/bench $1 $2 4 >$1.bench.stdout 2>&1
}

function array_ts_fsa_noCompID {
  ./src/build/bench $1 $2 5 >$1.bench.stdout 2>&1
}

function array_ts_fsa_noCompWo {
  ./src/build/bench $1 $2 6 >$1.bench.stdout 2>&1
}

TOOLS="
array_ts_fsa
array_ts_fsa_noCuWo
array_ts_fsa_noBro
array_ts_fsa_noCompID
array_ts_fsa_noCompWo
xcdat
centrp
marisa
"
#morfologik_fsa5
#morfologik_cfsa2
#array_fsa
#dac_array_fsa

DATASET_DIR=data-sets

DATASETS="
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
$DATASET_DIR/kanda/enwiki-20150205.1000000.rnd_dict
"

RESULTS_DIR=results

for dataset in $DATASETS; do
  for tool in $TOOLS; do
    dataset_fn=`basename $dataset .rnd_dict`
    dataset_fn=`basename $dataset_fn .1000000`
    echo "$tool, $dataset..."
    $tool $RESULTS_DIR/$dataset_fn/$dataset_fn.$tool $dataset
  done
done
