#!/bin/bash

function dam {
  ./build/daram_benchmark $1 $2 $3 >$1.bench.stdout 2>&1
}
export -f dam

function damcn {
  ./build/daram_benchmark $1 $2 $3 >$1.bench.stdout 2>&1
}
export -f damcn

function damci {
  ./build/daram_benchmark $1 $2 $3 >$1.bench.stdout 2>&1
}
export -f damci

function damcsi {
  ./build/daram_benchmark $1 $2 $3 >$1.bench.stdout 2>&1
}
export -f damcsi

function damdw {
  ./build/daram_benchmark $1 $2 $3 >$1.bench.stdout 2>&1
}
export -f damdw

function damac {
  ./build/daram_benchmark $1 $2 $3 >$1.bench.stdout 2>&1
}
export -f damac

function morfologik_fsa5d {
  ./build/experiment $1 $2 9 >$1.bench.stdout 2>&1
}
export -f morfologik_fsa5d

function morfologik_cfsa2d {
  ./build/experiment $1 $2 10 >$1.bench.stdout 2>&1
}
export -f morfologik_cfsa2d

function xcdat {
  ./build/experiment $1 $2 7 >$1.bench.stdout 2>&1
}
export -f xcdat

function fxcdat {
  ./build/experiment $1 $2 13 >$1.bench.stdout 2>&1
}
export -f fxcdat

function marisa {
  ./build/experiment $1 $2 8 >$1.bench.stdout 2>&1
}
export -f marisa

function darts {
  ./build/experiment $1 $2 12 >$1.bench.stdout 2>&1
}
export -f darts

function centrp {
  ./software/path_decomposed_tries/tries_perftest centroid_repair measure $1 $2 >$1.bench.stdout 2>&1
  cat $1.stdout >> $1.bench.stdout
}
export -f centrp


bench() {
    dataset_fn=`basename $2 .1000000.rnd_dict`
	dictname=$RESULTS_DIR/$dataset_fn/$dataset_fn.$1
	echo "benchmark [$1] $dictname... -> $3"
	$1 $dictname $2 $3
}
export -f bench

DARAM_TOOLS="
dam
damcn
damci
damcsi
damdw
"
export DARAM_TOOLS
#damac

MORFOLOGIK_TOOLS="
morfologik_fsa5d
morfologik_cfsa2d
"
export MORFOLOGIK_TOOLS

XCDAT_TOOLS="
xcdat
fxcdat
"
export XCDAT_TOOLS

CENT_TOOLS="
centrp
"
export CENT_TOOLS

MARISA_TOOLS="
marisa
"
export MARISA_TOOLS

DARTS_TOOLS="
darts
"
export DARTS_TOOLS

experiments() {
	dataset_fn=`basename $2 .1000000.rnd_dict`
	results_name=$RESULTS_DIR/$dataset_fn/$dataset_fn.$1
	echo "" > $results_name
	parallel bench {1} $2 $results_name ::: $1
}
export -f experiments

daram_results() {
	dataset_fn=`basename $1 .1000000.rnd_dict`
	results_name=$RESULTS_DIR/$dataset_fn/$dataset_fn.daram_results
	echo "" > $results_name
	parallel bench {1} $1 $results_name ::: $DARAM_TOOLS
}
export -f daram_results

morfologik_results() {
	dataset_fn=`basename $1 .1000000.rnd_dict`
	results_name=$RESULTS_DIR/$dataset_fn/$dataset_fn.morfologik_results
	echo "" > $results_name
	parallel bench {1} $1 $results_name ::: $MORFOLOGIK_TOOLS
}
export -f morfologik_results

xcdat_results() {
	dataset_fn=`basename $1 .1000000.rnd_dict`
	results_name=$RESULTS_DIR/$dataset_fn/$dataset_fn.xcdat_results
	echo "" > $results_name
	parallel bench {1} $1 $results_name ::: $XCDAT_TOOLS
}
export -f xcdat_results

cent_results() {
	dataset_fn=`basename $1 .1000000.rnd_dict`
	results_name=$RESULTS_DIR/$dataset_fn/$dataset_fn.cent_results
	echo "" > $results_name
	parallel bench {1} $1 $results_name ::: $CENT_TOOLS
}
export -f cent_results

marisa_results() {
	dataset_fn=`basename $1 .1000000.rnd_dict`
	results_name=$RESULTS_DIR/$dataset_fn/$dataset_fn.marisa_results
	echo "" > $results_name
	parallel bench {1} $1 $results_name ::: $MARISA_RESULTS
}
export -f marisa_results

darts_results() {
	dataset_fn=`basename $1 .1000000.rnd_dict`
	results_name=$RESULTS_DIR/$dataset_fn/$dataset_fn.darts_results
	echo "" > $results_name
	parallel bench {1} $1 $results_name ::: $DARTS_RESULTS
}
export -f darts_results

experiments() {
	$1 $2
}
export -f experiments

TOOLS="
daram_results
morfologik_results
xcdat_results
cent_results
marisa_results
darts_results
"

DATASET_DIR=data-sets
DATASETS=`find $DATASET_DIR -name '*.1000000.rnd_dict'`

RESULTS_DIR=results
export RESULTS_DIR

echo '--- Benchmark targets ---'
for f in $DATASETS; do
	echo $f
done
echo '------'

#python $DATASET_DIR/create-test-datasets.py

THREADS=1
if [ $# -ge 1 ] ; then
	THREADS=$1
fi
echo 'Threads: '$THREADS

parallel -j $THREADS experiments {1} {2} ::: $TOOLS ::: $DATASETS

