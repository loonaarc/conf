<#
Purpose:
Starts the local safety-monitoring development/demo tools.

What it opens:
- Tasmota web UI for ESP #1
- Mosquitto MQTT broker
- MQTT Explorer
- openHAB
- openHAB Basic UI sitemap

Run from PowerShell:
.\conf\scripts\start_safety_monitor.ps1
#>

$tasmotaUrl = "http://192.168.43.223"
$basicUiUrl = "http://localhost:8080/basicui/app?sitemap=safety_monitor"

$mosquittoPath = "C:\Program Files\Mosquitto\mosquitto.exe"
$mqttExplorerPath = "C:\Users\lil\AppData\Local\Programs\MQTT-Explorer\MQTT Explorer.exe"
$openHabRoot = "C:\Users\lil\openhab-5.1.4"
$openHabStart = Join-Path $openHabRoot "start.bat"

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

Write-Host "Opening Tasmota web UI..."
Start-Process $tasmotaUrl

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

Start-IfExists -Name "MQTT Explorer" -Path $mqttExplorerPath

if (Test-Path -LiteralPath $openHabStart) {
    Write-Host "Starting openHAB in a visible Command Prompt window..."
    Start-Process cmd.exe -ArgumentList @(
        "/k",
        "cd /d `"$openHabRoot`" && start.bat"
    )
} else {
    Write-Warning "openHAB start.bat was not found at: $openHabStart"
}

Write-Host "Waiting briefly before opening the Basic UI..."
Start-Sleep -Seconds 15

Write-Host "Opening openHAB Basic UI..."
Start-Process $basicUiUrl

Write-Host "Done. Keep the Mosquitto and openHAB windows open while testing."
