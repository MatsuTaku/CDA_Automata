#!/bin/bash

TIME_CMD=gtime
TIME_FMT="wall\t%e\t%E\tuser\t%U\tsys\t%S\tmax-mem-kb\t%M\tdta\t%D\tinputs\t%I\toutputs\t%O"

function array_fsa {
  $TIME_CMD -f $TIME_FMT -o $2.log \
  ./array-fsa/build/build $1 $2 >$2.stdout 2>&1

  cat $2.stdout
}

TOOLS="
array_fsa
"

DATASET_DIR=data-sets

DATASETS="
$DATASET_DIR/ciura-deorowicz/deutsch.dict
"

RESULTS_DIR=results-try

CURRENT_DIR=$(dirname $0)

cd $CURRENT_DIR
./01-compile-array-fsa.sh

for dataset in $DATASETS; do
  for tool in $TOOLS; do
    dataset_fn=`basename $dataset .dict`
    echo "$tool, $dataset..."
    mkdir -p $RESULTS_DIR/$dataset_fn
    $tool $dataset $RESULTS_DIR/$dataset_fn/$dataset_fn.$tool
  done
done
