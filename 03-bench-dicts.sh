#!/bin/bash

function dam {
  ./daram/build/tools/daram_benchmark $1 $2 $3 >$1.bench.stdout 2>&1
}
export -f dam

function damcn {
  ./daram/build/tools/daram_benchmark $1 $2 $3 >$1.bench.stdout 2>&1
}
export -f damcn

function damci {
  ./daram/build/tools/daram_benchmark $1 $2 $3 >$1.bench.stdout 2>&1
}
export -f damci

function damcsi {
  ./daram/build/tools/daram_benchmark $1 $2 $3 >$1.bench.stdout 2>&1
}
export -f damcsi

function damdw {
  ./daram/build/tools/daram_benchmark $1 $2 $3 >$1.bench.stdout 2>&1
}
export -f damdw

function damac {
  ./daram/build/tools/daram_benchmark $1 $2 $3 >$1.bench.stdout 2>&1
}
export -f damac

function morfologik_fsa5d {
  ./experiments/build/sd_experiments $1 $2 1 $3 >$1.bench.stdout 2>&1
}
export -f morfologik_fsa5d

function morfologik_cfsa2d {
  ./experiments/build/sd_experiments $1 $2 2 $3 >$1.bench.stdout 2>&1
}
export -f morfologik_cfsa2d

function xcdat {
  ./experiments/build/sd_experiments $1 $2 3 $3 >$1.bench.stdout 2>&1
}
export -f xcdat

function fxcdat {
  ./experiments/build/sd_experiments $1 $2 4 $3 >$1.bench.stdout 2>&1
}
export -f fxcdat

function marisa {
  ./experiments/build/sd_experiments $1 $2 5 $3 >$1.bench.stdout 2>&1
}
export -f marisa

function darts {
  ./experiments/build/sd_experiments $1 $2 6 $3 >$1.bench.stdout 2>&1
}
export -f darts

function centrp {
  #./software/path_decomposed_tries/tries_perftest centroid_repair measure $1 $2 >$1.bench.stdout 2>&1
  #cat $1.stdout >> $1.bench.stdout
	./experiments/build/sd_experiments $1 $2 7 $3 >$1.bench.stdout 2>&1
}
export -f centrp

function samc {
  ./experiments/build/sd_experiments $1 $2 8 $3 >$1.bench.stdout 2>&1
}
export -f samc


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

SAMC_TOOLS="
samc
"
export SAMC_TOOLS


bench() {
    dataset_fn=`basename $2 .1000000.rnd_dict`
	dictname=$RESULTS_DIR/$dataset_fn/$dataset_fn.$1
	echo "benchmark [$1] $dictname"
	$1 $dictname $2 $3
}
export -f bench

daram_results() {
	dataset_fn=`basename $1 .1000000.rnd_dict`
	results_path=$RESULTS_DIR/$dataset_fn/$dataset_fn.daram_results
	echo -n > $results_path
    echo "Experiments for each species -> ${results_path}"
    for tool in $DARAM_TOOLS; do
	    bench $tool $1 $results_path
    done
}
export -f daram_results

morfologik_results() {
dataset_fn=`basename $1 .1000000.rnd_dict`
	results_path=$RESULTS_DIR/$dataset_fn/$dataset_fn.morfologik_results
	echo -n > $results_path
    echo "Experiments for each species -> ${results_path}"
    for tool in $MORFOLOGIK_TOOLS; do
	    bench $tool $1 $results_path
    done
}
export -f morfologik_results

xcdat_results() {
	dataset_fn=`basename $1 .1000000.rnd_dict`
	results_path=$RESULTS_DIR/$dataset_fn/$dataset_fn.xcdat_results
	echo -n > $results_path
    echo "Experiments for each species -> ${results_path}"
    for tool in $XCDAT_TOOLS; do
	    bench $tool $1 $results_path
    done
}
export -f xcdat_results

cent_results() {
	dataset_fn=`basename $1 .1000000.rnd_dict`
	results_path=$RESULTS_DIR/$dataset_fn/$dataset_fn.cent_results
	echo -n > $results_path
    echo "Experiments for each species -> ${results_path}"
    for tool in $CENT_TOOLS; do
	    bench $tool $1 $results_path
    done
}
export -f cent_results

marisa_results() {
	dataset_fn=`basename $1 .1000000.rnd_dict`
	results_path=$RESULTS_DIR/$dataset_fn/$dataset_fn.marisa_results
	echo -n > $results_path
    echo "Experiments for each species -> ${results_path}"
    for tool in $MARISA_TOOLS; do
	    bench $tool $1 $results_path
    done
}
export -f marisa_results

darts_results() {
	dataset_fn=`basename $1 .1000000.rnd_dict`
	results_path=$RESULTS_DIR/$dataset_fn/$dataset_fn.darts_results
	echo -n > $results_path
    echo "Experiments for each species -> ${results_path}"
    for tool in $DARTS_TOOLS; do
	    bench $tool $1 $results_path
    done
}
export -f darts_results

samc_results() {
	dataset_fn=`basename $1 .1000000.rnd_dict`
	results_path=$RESULTS_DIR/$dataset_fn/$dataset_fn.samc_results
	echo -n > $results_path
    echo "Experiments for each species -> ${results_path}"
    for tool in $SAMC_TOOLS; do
	    bench $tool $1 $results_path
    done
}
export -f samc_results

call() {
	$1 $2
}
export -f call

TOOLS="
daram_results
morfologik_results
cent_results
darts_results
xcdat_results
marisa_results
"
#samc_results

DATASET_DIR=data-sets
DATASETS_LOCAL=`find $DATASET_DIR/local -name '*.1000000.rnd_dict'`
DATASETS_CIURA=`find $DATASET_DIR/ciura-deorowicz -name '*.1000000.rnd_dict'`
DATASETS_WEISS=`find $DATASET_DIR/weiss -name '*.1000000.rnd_dict'`
DATASETS="
$DATASETS_LOCAL
"
#$DATASETS_WEISS
#$DATASETS_CIURA

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

parallel -j $THREADS call {1} {2} ::: $TOOLS ::: $DATASETS

