# run_parallel.ps1 — winflexbison "our own" resistance test.
#
# Hammers the tool (win_flex/win_bison) with $Jobs background workers, each
# looping $Iterations times over the same grammar, all sharing %TEMP%. The test
# is self-verifying and FAILS unless all three hold:
#   1. Correctness — every run exits 0 and produces output byte-identical to a
#      single-call reference (a temp-file collision would corrupt output).
#   2. Concurrency — the peak number of the tool's temp files seen in %TEMP%
#      during the run rises >= $MinConcurrency above baseline, proving the runs
#      really overlapped (otherwise the test isn't exercising parallelism).
#   3. Cleanup — no net temp files are left in %TEMP% afterwards.
# This exercises the port's per-process-unique temp names (pid_tempname) and
# delete-on-close temp files.
[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$Tool,
    [Parameter(Mandatory)][string]$Grammar,
    [int]$Jobs = 8,
    [int]$Iterations = 25,
    [string[]]$ToolArgs = @(),
    [int]$MinConcurrency = 2
)
$ErrorActionPreference = 'Stop'
$Tool    = (Resolve-Path $Tool).Path
$Grammar = (Resolve-Path $Grammar).Path

# Only count the temp-file name prefixes THIS tool creates, so two of these
# tests can run concurrently (ctest -j) without cross-counting.
$leaf = Split-Path $Tool -Leaf
$patterns = if ($leaf -match 'bison') { @('~m4_in_*', '~m4_out_*') }
            else { @('~0_flex_*', '~flex_hdr_*', '~flex_out_main_*') }
function TempCount {
    $n = 0
    foreach ($p in $patterns) { $n += @(Get-ChildItem $env:TEMP -Filter $p -File -ErrorAction SilentlyContinue).Count }
    $n
}

$root = Join-Path $env:TEMP ('wfb_mc_' + [Guid]::NewGuid().ToString('N'))
try {
    # Reference: one sequential call.
    $refDir = Join-Path $root 'ref'; New-Item -ItemType Directory $refDir -Force | Out-Null
    Push-Location $refDir
    & $Tool @ToolArgs -o out.c $Grammar 1>$null 2>$null; $rc = $LASTEXITCODE
    Pop-Location
    if ($rc -ne 0) { Write-Host "reference run failed (exit $rc)"; exit 1 }
    $refHash = (Get-FileHash (Join-Path $refDir 'out.c') -Algorithm SHA256).Hash

    $worker = {
        param($tool, $targs, $grammar, $iter, $dir, $refHash)
        New-Item -ItemType Directory -Path $dir -Force | Out-Null
        Set-Location $dir
        for ($k = 0; $k -lt $iter; $k++) {
            & $tool @targs -o out.c $grammar 1>$null 2>$null
            if ($LASTEXITCODE -ne 0) { return "exit=$LASTEXITCODE@iter$k" }
            if ((Get-FileHash out.c -Algorithm SHA256).Hash -ne $refHash) { return "mismatch@iter$k" }
        }
        'ok'
    }

    $baseline = TempCount
    $js = for ($i = 0; $i -lt $Jobs; $i++) {
        Start-Job $worker -ArgumentList $Tool, $ToolArgs, $Grammar, $Iterations, (Join-Path $root "p$i"), $refHash
    }
    # Sample %TEMP% while the workers run to measure real concurrency.
    $peak = $baseline
    while (@($js | Where-Object { $_.State -eq 'Running' }).Count -gt 0) {
        $c = TempCount; if ($c -gt $peak) { $peak = $c }
        Start-Sleep -Milliseconds 8
    }
    $results = @($js | Receive-Job); $js | Remove-Job -Force
    Start-Sleep -Milliseconds 300
    $after = TempCount

    $bad         = @($results | Where-Object { $_ -ne 'ok' })
    $concurrency = $peak - $baseline
    $leak        = $after - $baseline

    Write-Host ("{0}: {1} jobs x {2} iters -> peak concurrency={3}, net temp leak={4}" -f $leaf, $Jobs, $Iterations, $concurrency, $leak)
    $ok = $true
    if ($bad.Count) { Write-Host "  FAIL: $($bad.Count) bad run(s): $(($bad | Sort-Object -Unique) -join ', ')"; $ok = $false }
    if ($concurrency -lt $MinConcurrency) { Write-Host "  FAIL: concurrency $concurrency < $MinConcurrency (runs did not overlap)"; $ok = $false }
    if ($leak -gt 0) { Write-Host "  FAIL: $leak temp file(s) leaked into %TEMP%"; $ok = $false }
    if (-not $ok) { exit 1 }
    Write-Host "  OK"
    exit 0
}
finally {
    Remove-Item $root -Recurse -Force -ErrorAction SilentlyContinue
}
