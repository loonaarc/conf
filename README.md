# Distributed Edge-Based Safety Monitoring Configuration

This folder contains the openHAB configuration for the distributed edge-based safety monitoring project. Use this README to start the system, open the UI, and troubleshoot the most common startup problems.

## What Is Included

- `things/mqtt.things` defines the MQTT broker and D1 Mini MQTT channels.
- `items/safety_monitor.items` defines the openHAB items shown in the UI and used by rules.
- `rules/safety_monitor.rules` currently toggles the relay when motion is detected and automation is enabled.
- `sitemaps/safety_monitor.sitemap` defines the current Basic UI page.
- `services/addons.cfg` installs the required openHAB add-ons on startup.

For the system design and data flow, see [ARCHITECTURE.md](ARCHITECTURE.md).

For the larger safety-monitoring project plan, see [PROJECT_PLAN.md](PROJECT_PLAN.md).

For breadboard, sensor, and pin wiring notes, see [WIRING_GUIDE.md](WIRING_GUIDE.md).

For final grading requirements and missing deliverables, see [FINAL_CHECKLIST.md](FINAL_CHECKLIST.md).

For screenshots and photos collected during the build, see [EVIDENCE.md](EVIDENCE.md).

## Requirements

- openHAB 5.1.4 unpacked locally.
- Java available for openHAB.
- Mosquitto MQTT broker running on the same machine.
- A Tasmota device reachable by MQTT.
- The first renamed monitoring-node MQTT topic: `safety_monitor_1`.
- Visual Studio Code with the openHAB extension is useful for editing the configuration files.

The required openHAB add-ons are configured in `services/addons.cfg`:

```text
binding = mqtt
transformation = jsonpath
ui = basic
```

## Device Preparation

The D1 Mini was flashed with the Tasmota desktop flasher, not the browser-based web installer.

The relevant preparation steps from Lab 1 are:

1. Connect the D1 Mini with a USB data cable.
2. Open the Tasmota desktop flashing app.
3. Select the new COM port that appears after connecting the D1 Mini.
4. Flash the default English Tasmota firmware, for example `tasmota.bin`.
5. Configure Wi-Fi after flashing, either through the Tasmota hotspot or through the desktop app's configuration feature.
6. Open the Tasmota web interface by using the device IP address.
7. Configure the module and GPIOs for the attached hardware.
8. Configure MQTT in the Tasmota web interface with the Mosquitto broker address, username, and password if authentication is used.

For the relay shield, the lab material uses the relay input on `D1`. After configuration, the relay should be switchable from the Tasmota web interface or console before openHAB is added.

## Flash With Tasmotizer

Use this for D1 Mini boards that do not already run Tasmota.

1. Unplug the D1 Mini.
2. Open Tasmotizer.
3. Plug in the D1 Mini with a USB data cable.
4. Refresh the COM port list.
5. Select the new COM port that appeared.
6. Select the firmware image:

```text
tasmota.bin
```

7. If the board is unknown or previously used, enable erase before flashing.
8. Click:

```text
Tasmotize!
```

9. Wait until flashing is finished.

After flashing, configure Wi-Fi. There are two possible ways.

### Option A: Send Wi-Fi Config From Tasmotizer

This is the quicker method from the Lab 1 instructions.

1. Keep the D1 Mini connected after flashing.
2. In Tasmotizer, click:

```text
Send config
```

3. Enter the Wi-Fi network name and password.

For the lab network, the Wi-Fi SSID used by the first D1 was:

```text
realme C3
```

For the advanced node, the following `Send config` values were used successfully:

```text
Wi-Fi SSID:      realme C3
MQTT host:       192.168.43.26
MQTT port:       1883
MQTT topic:      safety_advanced_1
MQTT full topic: %prefix%/%topic%/
Friendly name:   Safety Advanced 1
MQTT user:       empty
MQTT password:   empty
```

After sending the config, the node was found with:

```text
IP address: 192.168.43.240
```

4. Save/send the configuration.
5. Wait for the D1 Mini to restart and join the Wi-Fi.
6. Check the router/hotspot device list or scan the network to find the new IP address.

### Option B: Configure Through Tasmota Hotspot

If `Send config` does not work, use the temporary Tasmota access point.

1. After flashing, check your laptop Wi-Fi list.
2. Connect to the new Tasmota hotspot, usually named like:

```text
tasmota-xxxx
```

3. Open:

```text
http://192.168.4.1
```

4. Enter the Wi-Fi SSID and password.
5. Save and wait for the D1 Mini to restart.
6. Find the new IP address in the router/hotspot device list or by scanning the network.

## Power and Open Tasmota

Tasmota is the firmware running on the D1 Mini. There is no separate Tasmota program to start on the laptop. To make the Tasmota web interface available:

1. Power the D1 Mini with the USB cable.
2. Make sure the laptop is connected to the same network as the D1 Mini. In the lab this was the `192.168.43.x` network.
3. Open a browser on the laptop.
4. Go to the Tasmota device IP address:

```text
http://192.168.43.223/
```

In the lab, the device was identified as:

```text
IP address:  192.168.43.223
MAC address: A8:48:FA:C1:7D:DD
Hostname:    tasmota-C17DDD-7645
Topic:       safety_monitor_1
```

If the page does not open, check that the D1 Mini has power and still has the same IP address. The laptop IP was checked with:

```powershell
ipconfig
```

In the lab, the laptop address used as MQTT host was:

```text
192.168.43.26
```

The local network scan command used during the lab was:

```cmd
for /L %i in (2,1,254) do @ping -n 1 -w 10 192.168.43.%i >nul
```

## Pinout

For sensor and breadboard wiring details, including the DS18B20 temperature sensor, PIR sensor, and reed switch, see [WIRING_GUIDE.md](WIRING_GUIDE.md).

Project wiring convention:

```text
D1 Mini 3V3 -> breadboard red power rail
D1 Mini GND -> breadboard blue ground rail
```

Use 3.3V for sensor modules in this project unless a specific module explicitly requires 5V.

## Tasmota Module Settings

After the D1 Mini is powered and the Tasmota page is reachable, open:

```text
Configuration -> Configure Module
```

The relay shield uses:

```text
GPIO5 (D1) -> Relay1
```

For the current step-by-step build, this relay can remain on ESP #1 as the existing test actuator. Later, the final alarm relay/buzzer should be placed on ESP #2 so openHAB can demonstrate device 1 triggering device 2.


The current monitoring node also uses:

```text
GPIO2  (D4) -> DS18x20
GPIO4  (D2) -> Switch1
GPIO12 (D6) -> Switch2
```

The reed module has been verified with a magnet. If Tasmota reports `Switch2` as `TOGGLE`, the input is working. For a cleaner openHAB door item later, use follow mode:

```text
SwitchMode2 1
```

If the displayed state is backwards, use inverted follow mode:

```text
SwitchMode2 2
```

After saving, the device restarts.

Useful Tasmota console commands for the relay:

```text
POWER ON
POWER OFF
POWER TOGGLE
```

The commands are not case-sensitive.

If Tasmota shows `POWER ON/OFF` but the relay does not click, check the relay hardware wiring. When the relay shield was plugged directly on top of the D1 Mini, the shield pins likely provided signal, VCC, and GND automatically. On a breadboard, the relay needs its signal pin on `D1 / GPIO5`, plus the correct relay supply pin and shared GND.

To print the full Tasmota status report:

```text
status 0
```

Important values from the lab status output:

```text
DNS server:       192.168.43.1
MQTT user:        DVES_USER
Temperature:      27.9 C
TelePeriod:       300 seconds
Tasmota version:  15.3.0
```

## Start Mosquitto

Start Mosquitto before openHAB so the MQTT bridge can connect.

In this lab setup, Mosquitto could be started simply by opening the Mosquitto installation folder and double-clicking:

```text
mosquitto.exe
```

Leave the Mosquitto window open while openHAB is running.

On Windows, if Mosquitto is installed as a service:

```powershell
Get-Service mosquitto
Start-Service mosquitto
```

If you run Mosquitto manually, start it with its normal executable or service shortcut. The broker must listen on:

```text
localhost:1883
```

The following manual configuration is only needed if double-clicking `mosquitto.exe` does not work or if a specific listener configuration is required.

Create or use this file:

```text
C:\Programme\Mosquitto\mosquitto.conf
```

If Windows shows the English program folder name, the same location may appear as:

```text
C:\Program Files\Mosquitto
```

With this content:

```text
listener 1883
allow_anonymous true
```

If Mosquitto is already running as a service, stop it from an administrator terminal:

```cmd
net stop mosquitto
```

Then open a command prompt in the Mosquitto folder and start the broker with the config file:

```cmd
mosquitto -c mosquitto.conf -v
```

Optional broker test with two additional terminals:

```cmd
mosquitto_sub -t test
```

```cmd
mosquitto_pub -t test -m hello
```

The subscriber terminal should print:

```text
hello
```

## Check Tasmota MQTT Connection

After Mosquitto is running, check that Tasmota can connect to it before starting openHAB.

In the Tasmota web interface, configure MQTT under:

```text
Configuration -> Configure MQTT
```

Use:

```text
Host: 192.168.43.26
Port: 1883
User/password: empty, unless authentication is enabled
```

After saving, the Tasmota console should show:

```text
MQT: Attempting connection...
MQT: Connected
```

You can also use MQTT Explorer or the Mosquitto command-line clients to inspect the topics and publish a test command. This config expects these topics:

```text
Relay state:    stat/safety_monitor_1/POWER
Relay command:  cmnd/safety_monitor_1/POWER
Motion state:   stat/safety_monitor_1/RESULT
```

For the lab workflow, open MQTT Explorer after Tasmota says `MQT: Connected`:

1. Start Mosquitto.
2. Open MQTT Explorer.
3. Connect to the broker at `192.168.43.26` or `localhost`, port `1883`, depending on where MQTT Explorer is running.
4. Trigger the PIR or reed switch.
5. Check that `stat/safety_monitor_1/RESULT` changes.
6. Check that the working DS18x20 sensor publishes temperature under `tele/safety_monitor_1/SENSOR`.

Verified monitoring-node MQTT results:

```text
tele/safety_monitor_1/LWT = Online
stat/safety_monitor_1/RESULT = {"Switch1":{"Action":"ON"}}
stat/safety_monitor_1/RESULT = {"Switch1":{"Action":"OFF"}}
stat/safety_monitor_1/RESULT includes Switch2 when the reed is triggered by a magnet
tele/safety_monitor_1/SENSOR includes Switch1, Switch2, and DS18B20 temperature
```

If only `Switch1` updates immediately, that is still useful progress: the PIR path works. The reed switch creates `Switch2` messages on `stat/safety_monitor_1/RESULT` only when its digital output changes. If telemetry shows `"Switch2":"OFF"` but no change event appears, check the reed module `DO` pin, magnet position, shared GND, and the `GPIO12 (D6) -> Switch2` Tasmota setting.

If your device has a different Tasmota topic, update `things/mqtt.things`.

The Lab 1 MQTT topic answers were:

```text
Connection status topic: tele/safety_monitor_1/LWT
Temperature data topic:  tele/safety_monitor_1/SENSOR
Command topic prefix:    cmnd/safety_monitor_1/
Relay command topic:     cmnd/safety_monitor_1/POWER
```

Everything below `tele/safety_monitor_1/` comes from the ESP8266/Tasmota device. Everything below `$SYS` is Mosquitto broker status information, not device data.

## Start openHAB

From the openHAB root folder, one level above this `conf` folder:

```powershell
cd C:\Users\lil\openhab-5.1.4
.\start.bat
```

Wait until openHAB finishes starting. The first startup can take longer because openHAB installs the configured add-ons.

Then open:

```text
http://localhost:8080
```

For the Basic UI sitemap, use:

```text
http://localhost:8080/basicui/app?sitemap=safety_monitor
```

## How To Use It

1. Open the Basic UI sitemap.
2. Use `Relay` to manually switch the Tasmota relay.
3. Watch `Motion` to see motion state updates from MQTT.
4. Enable `Motion automation` if motion should toggle the relay.

When `Motion automation` is ON and the `Motion` item changes to ON, the rule in `rules/safety_monitor.rules` toggles the relay.

## Console Checks

The project workflow still uses the openHAB console and logs for configuration checks.

You can connect to the console by SSH:

```powershell
ssh -p 8101 openhab@localhost
```

The initial password is usually:

```text
habopen
```

Useful console checks:

```text
openhab:things list
openhab:items list
openhab:links list
```

All things should be `ONLINE`. State changes can be checked in:

```text
C:\Users\lil\openhab-5.1.4\userdata\logs\events.log
```

## Troubleshooting

If the relay does not switch:

- Check that Mosquitto is running on `localhost:1883`.
- Check that the Tasmota device is connected to the same MQTT broker.
- Check that the topic in `things/mqtt.things` matches the Tasmota device topic. The monitoring node now uses `safety_monitor_1`.
- Check the openHAB log in `C:\Users\lil\openhab-5.1.4\userdata\logs\openhab.log`.
- Check `events.log` to see whether openHAB receives item state changes.

If the Basic UI page is missing:

- Confirm `ui = basic` is present in `services/addons.cfg`.
- Restart openHAB and wait for the add-on installation to finish.

If motion does not update:

- Confirm the JSONPATH transformation add-on is installed.
- Check the payload published to `stat/safety_monitor_1/RESULT`.
- Make sure the payload contains `Switch1.Action`, because the motion channel currently reads `$.Switch1.Action`.
