#!/bin/bash
numberOfRuns=10
classPath="build/classes/java/main"

./gradlew classes || exit 1
java -cp $classPath org.example.Generator

./scriptJ.sh sequential 1 1 $numberOfRuns
for readers in 2 3; do
    for threads in 4 8 16; do
        ./scriptJ.sh parallel $threads $readers $numberOfRuns
    done
done

echo "Varianta,Nr threads,Nr readers,Timp mediu (ms),Tseq/Tpar"
awk -F, '$4 == "Medie" { if ($1 == "sequential") sequentialTime = $5; printf "%s,%s,%s,%s,%.2f\n", $1, $2, $3, $5, sequentialTime / $5 }' outJ.csv
