#!/bin/sh

#open $(dirname $0)/results/*/*.bench.stdout

RESULTS="
daram_results
morfologik_results
marisa_results
cent_results
xcdat_results
darts_results
"

DATASET_DIR=`dirname $0`/data-sets
DATASETS=`find $DATASET_DIR -name '*.dict'`

RESULTS_DIR=`dirname $0`/results
for dataset in $DATASETS; do
	dataset_fn=`basename $dataset .dict`
	results_fn=$RESULTS_DIR/$dataset_fn/$dataset_fn
	python figurelize.py $results_fn.daram_results \
		$results_fn.morfologik_results \
		$results_fn.marisa_results \
		$results_fn.cent_results \
		$results_fn.xcdat_results \
	#	$results_fn.darts_results
done

open `find $RESULTS_DIR -name '*.plot.png'`


