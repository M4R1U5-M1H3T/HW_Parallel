$numberOfRuns = 10
$mpiInclude = "C:\Program Files (x86)\Microsoft SDKs\MPI\Include"
$mpiLibrary = "C:\Program Files (x86)\Microsoft SDKs\MPI\Lib\x64"

g++ -std=c++20 -O3 -march=native -mtune=native main.cpp -o main.exe -I $mpiInclude -L $mpiLibrary -lmsmpi
if ($LASTEXITCODE -ne 0) { exit 1 }
g++ -std=c++20 -O3 generator.cpp -o generator.exe
if ($LASTEXITCODE -ne 0) { exit 1 }

Write-Host "Test case: N1 = N2 = 16"
Set-Content Numar1.txt "16", "9999444444449999"
Set-Content Numar2.txt "16", "9999555555559999"
.\runAll.ps1 $numberOfRuns @(5) @(4)

$testCases = @(
    @{ Lengths = "10000 10000"; WorkerProcesses = @(5, 9, 17); ScatterProcesses = @() },
    @{ Lengths = "1000 1000"; WorkerProcesses = @(); ScatterProcesses = @(4, 8, 16) },
    @{ Lengths = "100 100000"; WorkerProcesses = @(5, 9, 17); ScatterProcesses = @(4, 8, 16) }
)

foreach ($testCase in $testCases) {
    Write-Host "Test case: N1 N2 =" $testCase.Lengths
    $testCase.Lengths.Split(" ") | .\generator.exe
    .\runAll.ps1 $numberOfRuns $testCase.WorkerProcesses $testCase.ScatterProcesses
}
