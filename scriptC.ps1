$variant = $args[0]
$processes = $args[1]
$numberOfRuns = $args[2]
$sendMode = $args[3]
$exeName = ".\main.exe"

$programArguments = @($variant)
$variantName = $variant
if ($sendMode) {
    $programArguments += $sendMode
    $variantName = "$variant-$sendMode"
}

$firstLength = (Get-Content Numar1.txt -TotalCount 1).Trim()
$secondLength = (Get-Content Numar2.txt -TotalCount 1).Trim()
$numbers = "${firstLength}x$secondLength"
$totalTime = 0
for ($run = 1; $run -le $numberOfRuns; $run++) {
    Write-Host "Rulare" $run
    $programOutput = @(mpiexec -n $processes $exeName @programArguments)
    $executionTime = $programOutput[$programOutput.length - 1]
    $result = $programOutput[$programOutput.length - 2]
    Write-Host $result $executionTime
    $totalTime += [double]$executionTime

    if (!(Test-Path outC.csv)) { Set-Content outC.csv 'Cifre (N1xN2),Varianta,Nr procese,Rulare,Timp executie (ms),Verificare' }
    Add-Content outC.csv "$numbers,$variantName,$processes,$run,$executionTime,$result"
}
$averageTime = $totalTime / $numberOfRuns
Write-Host "Timp de executie mediu:" $averageTime
Add-Content outC.csv "$numbers,$variantName,$processes,Medie,$averageTime,"
