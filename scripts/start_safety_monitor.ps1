<#
Purpose:
Starts the local safety-monitoring development/demo tools.

What it opens:
- Tasmota web UI for ESP #1 Safety Monitor
- Tasmota web UI for ESP #2 Safety Alarm
- Tasmota web UI for ESP #3 Safety Context
- Mosquitto MQTT broker
- MQTT Explorer
- openHAB
- openHAB Basic UI sitemap
- myopenHAB remote Basic UI sitemap

Run from PowerShell:
.\scripts\start_safety_monitor.ps1
#>

# Tasmota runs directly on the ESP8266 boards.
# Opening all three pages makes it easy to check GPIO settings, MQTT status,
# and console output during the demo.
$tasmotaUrls = @(
    @{ Name = "ESP #1 Safety Monitor"; Url = "http://192.168.43.223" },
    @{ Name = "ESP #2 Safety Alarm"; Url = "http://192.168.43.110" },
    @{ Name = "ESP #3 Safety Context"; Url = "http://192.168.43.240" }
)

# Basic UI page generated from sitemaps/safety_monitor.sitemap.
$basicUiUrl = "http://localhost:8080/basicui/app?sitemap=safety_monitor"
$remoteBasicUiUrl = "https://myopenhab.org/basicui/app?sitemap=safety_monitor"

# Local tool paths used in this Windows lab setup.
$mosquittoPath = "C:\Program Files\Mosquitto\mosquitto.exe"
$mqttExplorerPath = "C:\Users\lil\AppData\Local\Programs\MQTT-Explorer\MQTT Explorer.exe"
$openHabRoot = "C:\Users\lil\openhab-5.1.4"
$openHabStart = Join-Path $openHabRoot "start.bat"

# Helper for optional GUI tools. If a tool is missing, the script keeps going
# and prints a warning instead of stopping the whole startup flow.
function Start-IfExists {
    param(
        [string]$Name,
        [string]$Path,
        [string]$WorkingDirectory = ""
    )

    if (Test-Path -LiteralPath $Path) {
        Write-Host "Starting $Name..."
        if ($WorkingDirectory -ne "") {
            Start-Process -FilePath $Path -WorkingDirectory $WorkingDirectory
        } else {
            Start-Process -FilePath $Path
        }
    } else {
        Write-Warning "$Name was not found at: $Path"
    }
}

Write-Host "Opening Tasmota web UIs..."
foreach ($device in $tasmotaUrls) {
    Write-Host "Opening $($device.Name): $($device.Url)"
    Start-Process $device.Url
}

# Mosquitto is the MQTT broker between Tasmota and openHAB.
# It is started in a visible window so connection messages are easy to see.
if (Test-Path -LiteralPath $mosquittoPath) {
    Write-Host "Starting Mosquitto in a visible PowerShell window..."
    Start-Process powershell.exe -ArgumentList @(
        "-NoExit",
        "-Command",
        "& '$mosquittoPath'"
    )
} else {
    Write-Warning "Mosquitto was not found at: $mosquittoPath"
}

Start-Sleep -Seconds 2

# MQTT Explorer is used as the raw MQTT debug/evidence view.
Start-IfExists -Name "MQTT Explorer" -Path $mqttExplorerPath

# openHAB is started from its root directory because start.bat expects that
# working directory layout.
if (Test-Path -LiteralPath $openHabStart) {
    Write-Host "Starting openHAB in a visible Command Prompt window..."
    Start-Process cmd.exe -ArgumentList @(
        "/k",
        "cd /d `"$openHabRoot`" && start.bat"
    )
} else {
    Write-Warning "openHAB start.bat was not found at: $openHabStart"
}

# Give openHAB a little time to start before opening the browser page.
Write-Host "Waiting briefly before opening the Basic UI..."
Start-Sleep -Seconds 15

Write-Host "Opening local openHAB Basic UI..."
Start-Process $basicUiUrl

Write-Host "Opening myopenHAB remote Basic UI..."
Start-Process $remoteBasicUiUrl

Write-Host "Done. Keep the Mosquitto and openHAB windows open while testing."
