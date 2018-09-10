#!/bin/bash

TIME_CMD=gtime
TIME_FMT="wall\t%e\t%E\tuser\t%U\tsys\t%S\tmax-mem-kb\t%M\tdta\t%D\tinputs\t%I\toutputs\t%O"

function plain {
  $TIME_CMD -f $TIME_FMT -o $2.log \
  ./src/build/makefsa $1 $2 >$2.stdout 2>&1
}


TOOLS="
plain
"

DATASET_DIR=data-sets

DATASETS="
$DATASET_DIR/weiss/wikipedia.dict
$DATASET_DIR/weiss/wikipedia2.dict
$DATASET_DIR/kanda/jawiki-20150118.dict
$DATASET_DIR/kanda/indochina-2004.dict
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
$DATASET_DIR/kanda/enwiki-20150205.dict
$DATASET_DIR/kanda/word-over999.dict
"

RESULTS_DIR=results

for dataset in $DATASETS; do
    dataset_fn=`basename $dataset .dict`
    echo "plain, $dataset..."
    mkdir -p $RESULTS_DIR/$dataset_fn
    $plain $dataset $RESULTS_DIR/$dataset_fn/$dataset_fn.$plain
done
