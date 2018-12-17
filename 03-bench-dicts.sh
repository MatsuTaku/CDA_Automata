#!/bin/bash

function dam {
  ./build/dam_bench $1 $2 11 >$1.bench.stdout 2>&1
}
export -f dam

function damci {
  ./build/dam_bench $1 $2 15 >$1.bench.stdout 2>&1
}
export -f damci

function damac {
  ./build/dam_bench $1 $2 2 >$1.bench.stdout 2>&1
}
export -f damac

function morfologik_fsa5d {
  ./build/dam_bench $1 $2 9 >$1.bench.stdout 2>&1
}
export -f morfologik_fsa5d

function morfologik_cfsa2d {
  ./build/dam_bench $1 $2 10 >$1.bench.stdout 2>&1
}
export -f morfologik_cfsa2d

function xcdat {
  ./build/dam_bench $1 $2 7 >$1.bench.stdout 2>&1
}
export -f xcdat

function fxcdat {
  ./build/dam_bench $1 $2 13 >$1.bench.stdout 2>&1
}
export -f fxcdat

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
dam
"
damci
morfologik_fsa5d
morfologik_cfsa2d
xcdat
fxcdat
centrp
marisa
darts
#damac

DATASET_DIR=data-sets
DATASETS=`find $DATASET_DIR -name '*.1000000.rnd_dict'`

RESULTS_DIR=results
export RESULTS_DIR

bench() {
    dataset_fn=`basename $2 .1000000.rnd_dict`
	dictname=$RESULTS_DIR/$dataset_fn/$dataset_fn.$1
	echo "benchmark [$1] $dictname..."
	$1 $dictname $2
}
export -f bench



echo '--- Benchmark targets ---'
for f in $DATASETS; do
	echo $f
done
echo '------'

python $DATASET_DIR/create-test-datasets.py

THREADS=1
if [ $# -ge 1 ] ; then
	THREADS=$1
fi
echo 'Threads: '$THREADS
parallel -j $THREADS bench {1} {2} ::: $TOOLS ::: $DATASETS

