$numberOfRuns = $args[0]
$workerProcessCounts = $args[1]
$scatterProcessCounts = $args[2]

.\scriptC.ps1 sequential 1 $numberOfRuns
foreach ($processes in $workerProcessCounts) {
    .\scriptC.ps1 standard $processes $numberOfRuns
    .\scriptC.ps1 standard $processes 1 ssend
    .\scriptC.ps1 optimized $processes $numberOfRuns
    .\scriptC.ps1 optimized $processes 1 ssend
    .\scriptC.ps1 async $processes $numberOfRuns
}
foreach ($processes in $scatterProcessCounts) {
    .\scriptC.ps1 scatter $processes $numberOfRuns
    .\scriptC.ps1 scatter $processes 1 ssend
}
