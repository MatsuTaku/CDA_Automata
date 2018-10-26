#!/bin/bash

function dam {
  ./build/dam_bench $1 $2 11 >$1.bench.stdout 2>&1
}
export -f dam

function damac {
  ./build/dam_bench $1 $2 2 >$1.bench.stdout 2>&1
}
export -f damac

function morfologik_fsa5 {
  ./build/dam_bench $1 $2 9 >$1.bench.stdout 2>&1
}
export -f morfologik_fsa5

function morfologik_cfsa2 {
  ./build/dam_bench $1 $2 10 >$1.bench.stdout 2>&1
}
export -f morfologik_cfsa2

function xcdat {
  ./build/dam_bench $1 $2 7 >$1.bench.stdout 2>&1
}
export -f xcdat

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
damac
dam
morfologik_fsa5
morfologik_cfsa2
xcdat
centrp
marisa
darts
"

DATASET_DIR=data-sets
DAATASET_EXT='.1000000.rnd_dict'
export DATASET_EXT
DATASETS=`find $DATASET_DIR -name '*'$DATASET_EXT`

RESULTS_DIR=results
export RESULTS_DIR

bench() {
    dataset_fn=`basename $2 .$DATASET_EXT`
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
if [ $# >= 1 ] ; then
	THREADS=$1
fi
parallel -j $THREADS bench {1} {2} ::: $TOOLS ::: $DATASETS

