#!/bin/bash

TIME_CMD=gtime
TIME_FMT="wall\t%e\t%E\tuser\t%U\tsys\t%S\tmax-mem-kb\t%M\tdta\t%D\tinputs\t%I\toutputs\t%O"

function arr_check {
  $TIME_CMD -f $TIME_FMT -o $2.log \
  ./array-fsa/build/build_array $1 $2 >$2.stdout 2>&1
}

TOOLS="
arr_check
"

DATASET_DIR=results

DATASETS="
$DATASET_DIR/wikipedia/wikipedia.array_ts_fsa
$DATASET_DIR/wikipedia2/wikipedia2.array_ts_fsa
$DATASET_DIR/jawiki-20150118/jawiki-20150118.array_ts_fsa
$DATASET_DIR/indochina-2004/indochina-2004.array_ts_fsa
$DATASET_DIR/enwiki-20150205/enwiki-20150205.array_ts_fsa
$DATASET_DIR/word-over999/word-over999.array_ts_fsa
"
#$DATASET_DIR/ciura-deorowicz/abc.array_ts_fsa
#$DATASET_DIR/ciura-deorowicz/deutsch.array_ts_fsa
#$DATASET_DIR/ciura-deorowicz/dimacs.array_ts_fsa
#$DATASET_DIR/ciura-deorowicz/enable.array_ts_fsa
#$DATASET_DIR/ciura-deorowicz/english.array_ts_fsa
#$DATASET_DIR/ciura-deorowicz/eo.array_ts_fsa
#$DATASET_DIR/ciura-deorowicz/esp.array_ts_fsa
#$DATASET_DIR/ciura-deorowicz/files.array_ts_fsa
#$DATASET_DIR/ciura-deorowicz/fr.array_ts_fsa
#$DATASET_DIR/ciura-deorowicz/full.array_ts_fsa
#$DATASET_DIR/ciura-deorowicz/ifiles.array_ts_fsa
#$DATASET_DIR/ciura-deorowicz/one.array_ts_fsa
#$DATASET_DIR/ciura-deorowicz/polish.array_ts_fsa
#$DATASET_DIR/ciura-deorowicz/random.array_ts_fsa
#$DATASET_DIR/ciura-deorowicz/russian.array_ts_fsa
#$DATASET_DIR/ciura-deorowicz/sample.array_ts_fsa
#$DATASET_DIR/ciura-deorowicz/scrable.array_ts_fsa
#$DATASET_DIR/ciura-deorowicz/test.array_ts_fsa
#$DATASET_DIR/ciura-deorowicz/unix_m.array_ts_fsa
#$DATASET_DIR/ciura-deorowicz/unix.array_ts_fsa
#$DATASET_DIR/ciura-deorowicz/webster.array_ts_fsa
#$DATASET_DIR/weiss/pl.array_ts_fsa

RESULTS_DIR=results

for dataset in $DATASETS; do
  for tool in $TOOLS; do
    dataset_fn=`basename $dataset .array_ts_fsa`
    echo "$tool, $dataset..."
    mkdir -p $RESULTS_DIR/$dataset_fn
    $tool $dataset $RESULTS_DIR/$dataset_fn/$dataset_fn.$tool
  done
done
