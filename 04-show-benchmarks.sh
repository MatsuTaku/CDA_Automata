#!/bin/sh

DATASET_DIR=`dirname $0`/data-sets
DATASETS=`find $DATASET_DIR -name '*.dict'`

RESULTS_DIR=`dirname $0`/results
export RESULTS_DIR
plot() {
    dataset=$1
	dataset_fn=`basename $dataset .dict`
	results_fn=$RESULTS_DIR/$dataset_fn/$dataset_fn
    results=`find $RESULTS_DIR/$dataset_fn -name '*.*_results'`
    ./figurelize.py $results
}
export -f plot

parallel -j 4 plot {1} ::: $DATASETS

open `find $RESULTS_DIR -name '*.plot.png'`


