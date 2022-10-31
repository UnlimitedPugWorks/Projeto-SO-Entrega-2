#!/bin/bash
echo Threads,exec_time,speedup > $2.speedups.csv
parTimestring=0
parTime=0
speedup=0
resParTime=0
cd ..
cd ..
cd Projeto1/CircuitRouter-SeqSolver
./CircuitRouter-SeqSolver $2
seqTimeString=$(grep -e "Elapsed time" $2.res)
seqTime=$(echo $seqTimeString | cut -c 15-24)
cd ..
cd ..
cd Projeto2/CircuitRouter-ParSolver
echo 1S, $seqTime, 1 >> $2.speedups.csv
echo "SeqSolver runtime="$seqTime
for ((i = 1; i <= $1; i++)){
		./CircuitRouter-ParSolver -t $i $2
		parTimestring=$(grep -e "Elapsed time" $2.res)
		parTime=$(echo $parTimestring | cut -c 15-24)
		echo "ParSolver runtime="$parTime
		speedup=$(echo "scale=6; ${seqTime}/${parTime}" | bc)
		echo "Speedup=" $speedup
		echo $i, $parTime ,$(echo "scale=6; ${seqTime}/${parTime}" | bc) >> $2.speedups.csv
}
