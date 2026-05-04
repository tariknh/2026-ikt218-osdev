$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Resolve-Path (Join-Path $ScriptDir "..\..\..")
$Iso = Join-Path $ProjectRoot "build\solo_alex94\kernel.iso"

Write-Host "=== IRQ Rush: Kernel Panic ==="
Write-Host "Project root: $ProjectRoot"
Write-Host "ISO:          $Iso"
Write-Host "Host OS:      Windows"
Write-Host ""

if (!(Test-Path $Iso)) {
    Write-Host "ERROR: kernel.iso was not found:"
    Write-Host "  $Iso"
    Write-Host ""
    Write-Host "Build first with:"
    Write-Host "  cmake --build build/solo_alex94 --target uiaos-create-image -j"
    exit 1
}

$QemuCommand = Get-Command "qemu-system-i386.exe" -ErrorAction SilentlyContinue
$Qemu = $null

if ($QemuCommand) {
    $Qemu = $QemuCommand.Source
} else {
    $PossiblePaths = @(
        "C:\Program Files\qemu\qemu-system-i386.exe",
        "C:\Program Files\QEMU\qemu-system-i386.exe",
        "C:\msys64\mingw64\bin\qemu-system-i386.exe",
        "$env:LOCALAPPDATA\Programs\QEMU\qemu-system-i386.exe"
    )

    foreach ($Path in $PossiblePaths) {
        if (Test-Path $Path) {
            $Qemu = $Path
            break
        }
    }
}

if (!$Qemu) {
    Write-Host "ERROR: Could not find native Windows QEMU."
    Write-Host "Use WSL/devcontainer route instead, or install QEMU for Windows."
    exit 1
}

Write-Host "Using native Windows QEMU:"
Write-Host "  $Qemu"
Write-Host "Audio backend: DirectSound"
Write-Host ""

& $Qemu `
    -boot d `
    -cdrom $Iso `
    -m 64 `
    -audiodev "dsound,id=snd0,latency=5000" `
    -machine "pcspk-audiodev=snd0"

exit $LASTEXITCODE
