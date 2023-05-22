#!/bin/bash

#benchmarks=("AT" "BW" "CM" "DP" "TA" "TL")
#benchmarks=("AT" "BW" "CM" "DP" "TA")
benchmarks=("CM")
for n in {1..15}; do
  for k in {1..15}; do
    for p in "${benchmarks[@]}"; do
      # Construct the parameter string
      path="fsp_automatas/$p/$p-$n-$k.txt"
      echo $path
      ./main < $path
    done
  done
done

# ./run.sh 2> /dev/null