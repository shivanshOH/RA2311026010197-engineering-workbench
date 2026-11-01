param([string]$Executable = '.\cmake-build\WorkbenchCLI.exe')
$ErrorActionPreference = 'Stop'
$exe = (Resolve-Path $Executable).Path
$root = Split-Path (Split-Path $exe -Parent) -Parent
$here = (Get-Location).Path
$demo = & $exe --demo 2>&1 | Out-String
if ($LASTEXITCODE -ne 0) { throw "Demo failed: $demo" }
foreach ($expected in @('elements=10 midpoint_max_error_C=1.25','elements=20 midpoint_max_error_C=0.3125','elements=40 midpoint_max_error_C=0.078125','elements=80 midpoint_max_error_C=0.01953125')) { if ($demo -notmatch [regex]::Escape($expected)) { throw "Missing convergence result: $expected" } }
if ($demo -notmatch 'finite_difference=') { throw 'Finite-difference results missing' }
$stl = Join-Path $here 'data\example_ascii.stl'
$csv = Join-Path $here 'data\synthetic_temperature.csv'
& $exe --import-stl $stl | Out-Null; if ($LASTEXITCODE -ne 0) { throw 'Valid STL failed' }
& $exe --import-csv $csv | Out-Null; if ($LASTEXITCODE -ne 0) { throw 'Valid CSV failed' }
$temp = Join-Path $here 'tests\tmp'; New-Item -ItemType Directory -Force $temp | Out-Null
$empty = Join-Path $temp 'empty.stl'; [IO.File]::WriteAllText($empty,'')
& $exe --import-stl $empty 2>&1 | Out-Null; if ($LASTEXITCODE -eq 0) { throw 'Empty STL was accepted' }
$bad = Join-Path $temp 'malformed.stl'; [IO.File]::WriteAllText($bad,"solid x`n facet normal 0 0 1`n vertex NaN 0 0`nendsolid`n")
& $exe --import-stl $bad 2>&1 | Out-Null; if ($LASTEXITCODE -eq 0) { throw 'Non-finite/malformed STL was accepted' }
$wrong = Join-Path $temp 'wrong.csv'; [IO.File]::WriteAllText($wrong,"node_id,x,y,z,t`n1,0,0,0,20`n")
& $exe --import-csv $wrong 2>&1 | Out-Null; if ($LASTEXITCODE -eq 0) { throw 'Wrong CSV schema was accepted' }
$nan = Join-Path $temp 'nan.csv'; [IO.File]::WriteAllText($nan,"node_id,x_m,y_m,z_m,temperature_C`n1,0,0,0,NaN`n")
& $exe --import-csv $nan 2>&1 | Out-Null; if ($LASTEXITCODE -eq 0) { throw 'Non-finite temperature was accepted' }
$large = Join-Path $temp 'too_large.csv'; $fs=[IO.File]::Create($large); $fs.SetLength(20MB+1); $fs.Dispose()
& $exe --import-csv $large 2>&1 | Out-Null; if ($LASTEXITCODE -eq 0) { throw 'Over-limit CSV was accepted' }
$bench = & $exe --bench 1000 40 2>&1 | Out-String
if ($LASTEXITCODE -ne 0) { throw "Serial/parallel agreement failed: $bench" }
if ($bench -notmatch 'speedup=') { throw 'Speedup and efficiency rows missing' }
Remove-Item -LiteralPath $temp -Recurse -Force
Write-Output 'PASS: FE convergence, finite-difference gradients, valid imports, empty/malformed/non-finite/schema/size rejection, and serial/parallel agreement.'
Write-Output $demo
Write-Output $bench
