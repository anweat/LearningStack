param(
    [string]$SourceSdkDir = "D:\OpenGL_SDK"
)

# Determine workspace root (parent of util folder)
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$workspaceRoot = Split-Path $scriptDir -Parent

$destLibDir = Join-Path $workspaceRoot "lib"
$destIncludeDir = Join-Path $workspaceRoot "include"

Write-Host "================================================" -ForegroundColor Cyan
Write-Host "OpenGL SDK Copy Utility" -ForegroundColor Cyan
Write-Host "================================================" -ForegroundColor Cyan
Write-Host ""

# Validate SDK directory exists
if (!(Test-Path $SourceSdkDir)) {
    Write-Host "ERROR: Source SDK directory not found: $SourceSdkDir" -ForegroundColor Red
    Write-Host "Please download OpenGL SDK to: $SourceSdkDir" -ForegroundColor Yellow
    exit 1
}

Write-Host "Copying OpenGL libraries and includes:" -ForegroundColor Green
Write-Host "  Source SDK: $SourceSdkDir" -ForegroundColor Green
Write-Host "  Dest lib: $destLibDir" -ForegroundColor Green
Write-Host "  Dest include: $destIncludeDir" -ForegroundColor Green
Write-Host ""

# Create destination directories if they don't exist
if (!(Test-Path $destLibDir)) { 
    New-Item -ItemType Directory -Path $destLibDir | Out-Null
    Write-Host "[+] Created directory: $destLibDir" -ForegroundColor Yellow 
}
if (!(Test-Path $destIncludeDir)) { 
    New-Item -ItemType Directory -Path $destIncludeDir | Out-Null
    Write-Host "[+] Created directory: $destIncludeDir" -ForegroundColor Yellow 
}

# Copy library files
$libraries = @(
    "glfw3.lib",
    "glfw3dll.lib",
    "glfw3_mt.lib",
    "glfw3.dll"
)

Write-Host ""
Write-Host "Copying library files..." -ForegroundColor Cyan
$libCount = 0
foreach ($lib in $libraries) {
    $sourcePath = Join-Path -Path (Join-Path $SourceSdkDir 'lib') -ChildPath $lib
    $destPath = Join-Path $destLibDir $lib
    if (Test-Path $sourcePath) {
        Copy-Item $sourcePath $destPath -Force
        Write-Host "[✓] Copied: $lib" -ForegroundColor Green
        $libCount++
    } else {
        Write-Host "[✗] Not found: $lib" -ForegroundColor Yellow
    }
}

if ($libCount -eq 0) {
    Write-Host "WARNING: No library files were found or copied!" -ForegroundColor Yellow
} else {
    Write-Host "[+] Copied $libCount library files" -ForegroundColor Green
}

# Copy include tree (glad, GLFW, KHR, etc.) from SDK include to workspace include
$sourceIncludeDir = Join-Path $SourceSdkDir 'include'
Write-Host ""
Write-Host "Copying include directory tree..." -ForegroundColor Cyan
$includeCount = 0
if (Test-Path $sourceIncludeDir) {
    Get-ChildItem -Path $sourceIncludeDir -Directory | ForEach-Object {
        $sub = $_.Name
        $src = Join-Path $sourceIncludeDir $sub
        $dst = Join-Path $destIncludeDir $sub
        Copy-Item -Path $src -Destination $dst -Recurse -Force -ErrorAction SilentlyContinue
        Write-Host "[✓] Copied include folder: $sub" -ForegroundColor Green
        $includeCount++
    }
} else {
    Write-Host "[✗] Source include directory not found: $sourceIncludeDir" -ForegroundColor Red
    exit 1
}

# Copy glad.c if present at SDK root into include\glad\glad.c
$sourceGladC = Join-Path $SourceSdkDir 'glad.c'
if (Test-Path $sourceGladC) {
    $destGladDir = Join-Path $destIncludeDir 'glad'
    if (!(Test-Path $destGladDir)) { New-Item -ItemType Directory -Path $destGladDir | Out-Null }
    Copy-Item $sourceGladC (Join-Path $destGladDir 'glad.c') -Force
    Write-Host "[✓] Copied glad.c to $destGladDir" -ForegroundColor Green
}

# Verify key files exist
Write-Host ""
Write-Host "Verifying copied files..." -ForegroundColor Cyan
$verifyCount = 0
$requiredFiles = @(
    "lib\glfw3dll.lib",
    "lib\glfw3.dll",
    "include\glad\glad.h",
    "include\glad\glad.c",
    "include\GLFW\glfw3.h",
    "include\KHR\khrplatform.h"
)

foreach ($relPath in $requiredFiles) {
    $fullPath = Join-Path $workspaceRoot $relPath
    if (Test-Path $fullPath) {
        Write-Host "[✓] $relPath" -ForegroundColor Green
        $verifyCount++
    } else {
        Write-Host "[✗] MISSING: $relPath" -ForegroundColor Yellow
    }
}

Write-Host ""
Write-Host "================================================" -ForegroundColor Cyan
if ($verifyCount -eq $requiredFiles.Count) {
    Write-Host "SUCCESS: All files copied and verified!" -ForegroundColor Green
    Write-Host "Project is ready to build and debug." -ForegroundColor Green
} else {
    Write-Host "WARNING: Some files may be missing ($verifyCount/$($requiredFiles.Count))" -ForegroundColor Yellow
}
Write-Host "================================================" -ForegroundColor Cyan
Write-Host ""
