#!/bin/bash

TIME_CMD=gtime
TIME_FMT="wall\t%e\t%E\tuser\t%U\tsys\t%S\tmax-mem-kb\t%M\tdta\t%D\tinputs\t%I\toutputs\t%O"

function morfologik_fsa5 {
  $TIME_CMD -f $TIME_FMT -o $3.log \
  java -server -Xms50m -jar software/cfsa2.nx/morfologik*.jar \
    fsa_build \
       --format fsa5 \
       --progress \
       --sorted -i $1 \
       -o $3 >$3.stdout 2>&1
}

function morfologik_cfsa2 {
  $TIME_CMD -f $TIME_FMT -o $3.log \
  java -server -Xms50m -jar software/cfsa2.nx/morfologik*.jar \
    fsa_build \
       --format cfsa2 \
       --progress \
       --sorted -i $1 \
       -o $3 >$3.stdout 2>&1
}

function array_fsa {
  $TIME_CMD -f $TIME_FMT -o $3.log \
  ./array-fsa/build/build $1 $2 $3 1 >$3.stdout 2>&1
}

function dac_array_fsa {
  $TIME_CMD -f $TIME_FMT -o $2.log \
  ./array-fsa/build/build $1 $2 $3 2 >$3.stdout 2>&1
}

function array_ts_fsa {
  $TIME_CMD -f $TIME_FMT -o $2.log \
  ./array-fsa/build/build $1 $2 $3 3 >$3.stdout 2>&1
}


TOOLS="
array_ts_fsa
"
morfologik_fsa5
morfologik_cfsa2
array_fsa
dac_array_fsa

DATASET_DIR=data-sets

DATASETS="
$DATASET_DIR/weiss/wikipedia.dict
$DATASET_DIR/weiss/wikipedia2.dict
$DATASET_DIR/kanda/jawiki-20150118.dict
$DATASET_DIR/kanda/indochina-2004.dict
$DATASET_DIR/kanda/enwiki-20150205.dict
$DATASET_DIR/kanda/word-over999.dict
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
$DATASET_DIR/weiss/pl.dict
"

RESULTS_DIR=results

for dataset in $DATASETS; do
  for tool in $TOOLS; do
    dataset_fn=`basename $dataset .dict`
    echo "$tool, $dataset..."
    mkdir -p $RESULTS_DIR/$dataset_fn
    $tool $dataset $RESULTS_DIR/$dataset_fn/$dataset_fn.plain $RESULTS_DIR/$dataset_fn/$dataset_fn.$tool
  done
done