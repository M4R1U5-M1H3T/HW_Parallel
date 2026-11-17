#!/bin/bash
variant=$1
threads=$2
readers=$3
numberOfRuns=$4
classPath="build/classes/java/main"

totalTime=0
for run in $(seq 1 $numberOfRuns); do
    echo "Rulare $run"
    programOutput=$(java -cp $classPath org.example.Main $variant $threads $readers)
    result=$(echo "$programOutput" | tail -n 2 | head -n 1)
    executionTime=$(echo "$programOutput" | tail -n 1)
    echo "$result $executionTime"
    totalTime=$(echo "$totalTime + $executionTime" | bc)

    if [ ! -f outJ.csv ]; then echo "Varianta,Nr threads,Nr readers,Rulare,Timp executie (ms),Verificare" > outJ.csv; fi
    echo "$variant,$threads,$readers,$run,$executionTime,$result" >> outJ.csv
done
averageTime=$(echo "scale=4; $totalTime / $numberOfRuns" | bc)
echo "Timp de executie mediu: $averageTime"
echo "$variant,$threads,$readers,Medie,$averageTime," >> outJ.csv
