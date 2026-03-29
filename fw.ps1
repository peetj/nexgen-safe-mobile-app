param(
  [Parameter(Position = 0)]
  [ValidateSet("build", "upload", "monitor", "package", "list-ports", "help")]
  [string]$Command = "help",

  [Parameter(Position = 1)]
  [ValidateSet("wifi", "ble")]
  [string]$Target = "wifi",

  [string]$Port,
  [string]$Board = "esp32:esp32:esp32",
  [string]$DeviceName,
  [string]$PartitionScheme,
  [switch]$Clean
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Write-Usage {
  @"
Firmware CLI wrapper for Nexgen Safe.

Usage:
  .\fw.ps1 build [wifi|ble] [-DeviceName NAME] [-Board FQBN] [-PartitionScheme NAME] [-Clean]
  .\fw.ps1 upload [wifi|ble] [-Port COM5] [-DeviceName NAME] [-Board FQBN] [-PartitionScheme NAME] [-Clean]
  .\fw.ps1 monitor [wifi|ble] [-Port COM5] [-Board FQBN]
  .\fw.ps1 package [wifi|ble] [-DeviceName NAME]
  .\fw.ps1 list-ports

Examples:
  .\fw.ps1 build
  .\fw.ps1 build wifi -PartitionScheme huge_app
  .\fw.ps1 build ble -DeviceName NexgenSafe-02
  .\fw.ps1 upload wifi -Port COM5
  .\fw.ps1 package wifi
  .\fw.ps1 monitor -Port COM5
"@ | Write-Host
}

function Get-ArduinoCliPath {
  $preferred = "C:\arduino-cli\arduino-cli.exe"
  if (Test-Path -LiteralPath $preferred) {
    return $preferred
  }

  $command = Get-Command arduino-cli -ErrorAction SilentlyContinue
  if ($null -ne $command) {
    return $command.Source
  }

  throw "arduino-cli was not found. Install it first or add it to PATH."
}

function Get-TargetConfig {
  param(
    [Parameter(Mandatory = $true)]
    [string]$Name
  )

  switch ($Name) {
    "wifi" {
      return @{
        Name = "wifi"
        SketchName = "Nexgen_Safe_BLE_WIFI"
        SketchFile = "Nexgen_Safe_BLE_WIFI.ino"
        PackageSketchName = "Nexgen_Safe_BLE_Wifi"
        PackageSketchFile = "Nexgen_Safe_BLE_Wifi.ino"
        PackageName = "nexgen-safe-wifi"
        DefaultPartitionScheme = "huge_app"
        Dependencies = @(
          "SafeState.h",
          "SafeState.cpp",
          "icons.h",
          "icons.cpp",
          "web_ui.h"
        )
      }
    }
    "ble" {
      return @{
        Name = "ble"
        SketchName = "Nexgen_Safe_BLE"
        SketchFile = "Nexgen_Safe_BLE.ino"
        PackageSketchName = "Nexgen_Safe_BLE"
        PackageSketchFile = "Nexgen_Safe_BLE.ino"
        PackageName = "nexgen-safe-ble"
        DefaultPartitionScheme = "default"
        Dependencies = @(
          "SafeState.h",
          "SafeState.cpp",
          "icons.h",
          "icons.cpp"
        )
      }
    }
    default {
      throw "Unknown target '$Name'."
    }
  }
}

function Resolve-PartitionScheme {
  param(
    [Parameter(Mandatory = $true)]
    [hashtable]$Config,

    [string]$RequestedPartitionScheme
  )

  if ($RequestedPartitionScheme) {
    return $RequestedPartitionScheme
  }

  return $Config.DefaultPartitionScheme
}

function Get-SerialPorts {
  return @(Get-CimInstance Win32_SerialPort -ErrorAction SilentlyContinue |
    Sort-Object DeviceID)
}

function Show-SerialPorts {
  $ports = @(Get-SerialPorts)
  if ($ports.Count -eq 0) {
    Write-Host "No serial ports detected."
    return
  }

  foreach ($serialPort in $ports) {
    $name = $serialPort.Name
    $deviceId = $serialPort.DeviceID
    Write-Host ("{0}`t{1}" -f $deviceId, $name)
  }
}

function Resolve-Port {
  param(
    [string]$RequestedPort
  )

  if ($RequestedPort) {
    return $RequestedPort
  }

  $ports = @(Get-SerialPorts)
  if ($ports.Count -eq 1) {
    return $ports[0].DeviceID
  }

  if ($ports.Count -eq 0) {
    throw "No serial ports detected. Plug in the ESP32 and rerun."
  }

  $available = ($ports | ForEach-Object { $_.DeviceID }) -join ", "
  throw "Multiple serial ports detected ($available). Pass -Port explicitly."
}

function Prepare-SketchWorkspace {
  param(
    [Parameter(Mandatory = $true)]
    [hashtable]$Config,

    [string]$NameOverride
  )

  $repoRoot = $PSScriptRoot
  $firmwareRoot = Join-Path $repoRoot "firmware"
  $workspaceRoot = Join-Path $repoRoot "build\firmware\sketches"
  $sketchRoot = Join-Path $workspaceRoot $Config.SketchName

  if (Test-Path -LiteralPath $sketchRoot) {
    Remove-Item -LiteralPath $sketchRoot -Recurse -Force
  }

  New-Item -ItemType Directory -Path $sketchRoot -Force | Out-Null

  $sourceSketchPath = Join-Path $firmwareRoot $Config.SketchFile
  if (-not (Test-Path -LiteralPath $sourceSketchPath)) {
    throw "Missing sketch file: $sourceSketchPath"
  }

  $sketchContent = Get-Content -LiteralPath $sourceSketchPath -Raw
  if ($NameOverride) {
    $escapedName = $NameOverride.Replace('\', '\\').Replace('"', '\"')
    $pattern = 'static const char\* DEVICE_NAME = ".*?";'
    $replacement = ('static const char* DEVICE_NAME = "{0}";' -f $escapedName)
    $match = [regex]::Match($sketchContent, $pattern)

    if (-not $match.Success) {
      throw "Could not apply DEVICE_NAME override in $sourceSketchPath"
    }

    $sketchContent = $sketchContent.Substring(0, $match.Index) + $replacement + $sketchContent.Substring($match.Index + $match.Length)
  }

  $targetSketchPath = Join-Path $sketchRoot $Config.SketchFile
  [System.IO.File]::WriteAllText($targetSketchPath, $sketchContent, [System.Text.UTF8Encoding]::new($false))

  foreach ($dependency in $Config.Dependencies) {
    $sourceDependency = Join-Path $firmwareRoot $dependency
    if (-not (Test-Path -LiteralPath $sourceDependency)) {
      throw "Missing dependency: $sourceDependency"
    }

    Copy-Item -LiteralPath $sourceDependency -Destination (Join-Path $sketchRoot $dependency)
  }

  return $sketchRoot
}

function Invoke-ArduinoCli {
  param(
    [Parameter(Mandatory = $true)]
    [string[]]$Arguments
  )

  $arduinoCli = Get-ArduinoCliPath
  & $arduinoCli @Arguments
  if ($LASTEXITCODE -ne 0) {
    throw ("arduino-cli exited with code {0}." -f $LASTEXITCODE)
  }
}

$config = $null
if ($Command -in @("build", "upload", "monitor", "package")) {
  $config = Get-TargetConfig -Name $Target
}

switch ($Command) {
  "help" {
    Write-Usage
  }

  "list-ports" {
    Show-SerialPorts
  }

  "build" {
    $workspace = Prepare-SketchWorkspace -Config $config -NameOverride $DeviceName
    $outputDir = Join-Path $PSScriptRoot ("build\firmware\out\{0}" -f $config.Name)
    $effectivePartitionScheme = Resolve-PartitionScheme -Config $config -RequestedPartitionScheme $PartitionScheme

    if ($Clean -and (Test-Path -LiteralPath $outputDir)) {
      Remove-Item -LiteralPath $outputDir -Recurse -Force
    }

    New-Item -ItemType Directory -Path $outputDir -Force | Out-Null

    $arguments = @(
      "compile",
      "--fqbn", $Board,
      "--board-options", ("PartitionScheme={0}" -f $effectivePartitionScheme),
      "--output-dir", $outputDir
    )

    if ($Clean) {
      $arguments += "--clean"
    }

    $arguments += $workspace

    Write-Host ("Building {0} firmware for {1}" -f $config.Name, $Board)
    Write-Host ("Using partition scheme: {0}" -f $effectivePartitionScheme)
    if ($DeviceName) {
      Write-Host ("Using DEVICE_NAME override: {0}" -f $DeviceName)
    }

    Invoke-ArduinoCli -Arguments $arguments
    Write-Host ("Build output: {0}" -f $outputDir)
  }

  "upload" {
    $resolvedPort = Resolve-Port -RequestedPort $Port
    $workspace = Prepare-SketchWorkspace -Config $config -NameOverride $DeviceName
    $outputDir = Join-Path $PSScriptRoot ("build\firmware\out\{0}" -f $config.Name)
    $effectivePartitionScheme = Resolve-PartitionScheme -Config $config -RequestedPartitionScheme $PartitionScheme

    if ($Clean -and (Test-Path -LiteralPath $outputDir)) {
      Remove-Item -LiteralPath $outputDir -Recurse -Force
    }

    New-Item -ItemType Directory -Path $outputDir -Force | Out-Null

    $arguments = @(
      "compile",
      "--fqbn", $Board,
      "--board-options", ("PartitionScheme={0}" -f $effectivePartitionScheme),
      "--output-dir", $outputDir,
      "--upload",
      "--port", $resolvedPort
    )

    if ($Clean) {
      $arguments += "--clean"
    }

    $arguments += $workspace

    Write-Host ("Uploading {0} firmware to {1} using {2}" -f $config.Name, $resolvedPort, $Board)
    Write-Host ("Using partition scheme: {0}" -f $effectivePartitionScheme)
    if ($DeviceName) {
      Write-Host ("Using DEVICE_NAME override: {0}" -f $DeviceName)
    }

    Invoke-ArduinoCli -Arguments $arguments
  }

  "monitor" {
    $resolvedPort = Resolve-Port -RequestedPort $Port
    $arguments = @(
      "monitor",
      "--port", $resolvedPort,
      "--fqbn", $Board,
      "--config", "baudrate=115200"
    )

    Write-Host ("Opening serial monitor on {0}" -f $resolvedPort)
    Invoke-ArduinoCli -Arguments $arguments
  }

  "package" {
    $workspace = Prepare-SketchWorkspace -Config $config -NameOverride $DeviceName
    $packagesDir = Join-Path $PSScriptRoot "build\firmware\packages"
    $stagingDir = Join-Path $packagesDir "staging"
    $packageSketchRoot = Join-Path $stagingDir $config.PackageSketchName
    $packageSketchFile = $config.PackageSketchFile
    $zipPath = Join-Path $packagesDir ("{0}.zip" -f $config.PackageName)

    New-Item -ItemType Directory -Path $packagesDir -Force | Out-Null
    if (Test-Path -LiteralPath $stagingDir) {
      Remove-Item -LiteralPath $stagingDir -Recurse -Force
    }
    New-Item -ItemType Directory -Path $stagingDir -Force | Out-Null

    if (Test-Path -LiteralPath $zipPath) {
      Remove-Item -LiteralPath $zipPath -Force
    }

    New-Item -ItemType Directory -Path $packageSketchRoot -Force | Out-Null
    Get-ChildItem -LiteralPath $workspace | ForEach-Object {
      if ($_.Name -ne $config.SketchFile) {
        Copy-Item -LiteralPath $_.FullName -Destination (Join-Path $packageSketchRoot $_.Name) -Recurse -Force
      }
    }

    $packagedSketchContent = Get-Content -LiteralPath (Join-Path $workspace $config.SketchFile) -Raw
    [System.IO.File]::WriteAllText((Join-Path $packageSketchRoot $packageSketchFile), $packagedSketchContent, [System.Text.UTF8Encoding]::new($false))

    Add-Type -AssemblyName System.IO.Compression.FileSystem
    [System.IO.Compression.ZipFile]::CreateFromDirectory(
      $packageSketchRoot,
      $zipPath,
      [System.IO.Compression.CompressionLevel]::Optimal,
      $true
    )
    Remove-Item -LiteralPath $packageSketchRoot -Recurse -Force

    Write-Host ("Packaged {0} firmware sketch to {1}" -f $config.Name, $zipPath)
    Write-Host ("Zip expands to sketch folder: {0}" -f $config.PackageSketchName)
    if ($DeviceName) {
      Write-Host ("Using DEVICE_NAME override: {0}" -f $DeviceName)
    }
  }
}
