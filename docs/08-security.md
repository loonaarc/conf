# Security Configuration

This project is a lab prototype. The current setup prioritizes local demonstrability, but the security-relevant choices are documented explicitly.

## Current Security Setup

| Area | Current configuration |
| --- | --- |
| Network | Local Wi-Fi/hotspot network, documented as `192.168.43.x` |
| Wi-Fi SSID used during setup | `realme C3` |
| openHAB host | Local Windows laptop running openHAB 5.1.4 |
| openHAB local URL | `http://localhost:8080` |
| Basic UI URL | `http://localhost:8080/basicui/app?sitemap=safety_monitor` |
| Remote openHAB access | myopenHAB Cloud Connector |
| MQTT broker | Mosquitto on the local laptop |
| MQTT port | `1883` |
| MQTT transport | Plain local MQTT, no TLS |
| MQTT broker access | Lab setup uses anonymous/empty credentials unless authentication is explicitly enabled |
| openHAB MQTT bridge | Connects to `localhost:1883` |
| Tasmota device access | Local Tasmota web UIs on the same Wi-Fi/hotspot network |
| Tasmota web UI password | No separate web UI password is currently required in the lab setup |
| HTTP webservice | GeoSphere Austria warning API through openHAB HTTP binding |
| Persistence | Local JDBC/SQLite database under `userdata/persistence/safety_monitor.db` |

## Device Security

The Tasmota nodes are reachable on the local lab network:

| Node | IP address | Topic | Role |
| --- | --- | --- | --- |
| ESP #1 Safety Monitor | `192.168.43.223` | `safety_monitor_1` | PIR, reed switch |
| ESP #2 Safety Alarm | `192.168.43.110` | `safety_alarm_1` | Relay, buzzer actuator, touch acknowledgement |
| ESP #3 Safety Context | `192.168.43.240` | `safety_context_1` | Vibration, analog sound, temperature |

Security-relevant notes:

- Tasmota web UIs should only be reachable from the local lab network.
- Tasmota web UIs should not be exposed directly to the internet.
- In the current lab setup, the Tasmota web UIs do not require a separate web password.
- For a real deployment, each Tasmota device should have a web UI password.
- The device topics use project-specific names instead of default generated names.
- MQTT LWT status is integrated into openHAB so offline devices are visible in the UI.

## MQTT Security

Current MQTT flow:

```text
Tasmota nodes
  -> local Wi-Fi/hotspot
  -> Mosquitto on laptop, port 1883
  -> openHAB MQTT binding
```

Current broker configuration documented in setup notes:

```text
listener 1883
allow_anonymous true
```

Current limitations:

- MQTT traffic is not encrypted.
- Anonymous MQTT is convenient for the lab, but weak for real deployments.
- Any client on the same network could potentially publish commands if the broker accepts anonymous connections.

Recommended improvements:

- Disable anonymous MQTT.
- Configure MQTT username/password.
- Add MQTT TLS if the broker is used beyond a trusted local lab network.
- Keep command topics such as `cmnd/safety_alarm_1/POWER` protected, because they control actuators.

## openHAB Security

Current openHAB access:

```text
http://localhost:8080
http://localhost:8080/basicui/app?sitemap=safety_monitor
```

Remote access:

```text
myopenHAB Cloud Connector
https://myopenhab.org/basicui/app?sitemap=safety_monitor
```

Security-relevant notes:

- openHAB can be used locally through `localhost`.
- Remote access is handled through myopenHAB instead of direct port forwarding.
- The project should not use direct public port forwarding to expose openHAB.
- The myopenHAB UUID and secret must not be committed, published, or shown in screenshots.
- The openHAB console uses SSH on port `8101`; default credentials should be changed in a real deployment.

## Persistence Security

The project uses local JDBC/SQLite persistence:

```text
userdata/persistence/safety_monitor.db
```

Security-relevant notes:

- The database is local to the openHAB machine.
- It stores sensor states, risk states, actuator states, and node availability.
- It should not be shared publicly because it can reveal behavior patterns, device availability, and safety events.
- TinyML history values later should store summarized inference outputs, not raw audio.

## External Webservice Security

The project reads GeoSphere Austria warning data through the openHAB HTTP binding:

```text
https://warnungen.zamg.at/wsapp/api/getWarningsForCoords?lon=16.3738&lat=48.2082&lang=en
```

Security-relevant notes:

- The webservice is read-only in this project.
- GeoSphere values are used as context/risk modifiers.
- If the HTTP request fails, the local safety system should still work from local sensor events.

## TinyML Privacy Note

The TinyML audio node should publish sound classes and confidence values instead of raw audio:

```text
raw audio stays local
label/confidence goes to MQTT
```

During development, raw sample previews or feature values may be inspected locally through serial output or temporary debug MQTT topics. The deployed openHAB integration should use summarized data only.

This is one of the strongest privacy arguments for the TinyML extension.

## Risk Summary

| Risk | Current handling | Better real-world setup |
| --- | --- | --- |
| MQTT without TLS | Local lab network only | MQTT TLS |
| Anonymous MQTT | Acceptable only for lab testing | MQTT username/password |
| Tasmota web UI exposure | Local network only | Device passwords and isolated IoT network |
| openHAB public exposure | myopenHAB Cloud Connector, no direct port forwarding | VPN or myopenHAB cloud |
| Raw audio privacy | No raw audio in normal MQTT path | TinyML summary payloads only |
| Device outage invisible | MQTT LWT shown in openHAB UI | Keep LWT and alert on offline nodes |
| Stored history data | Local SQLite database | Protect database and host account |

## Evidence To Capture

Recommended evidence screenshots:

```text
docs/evidence/23-mosquitto-local-broker-config.png
docs/evidence/24-tasmota-mqtt-config-safety-monitor.png
docs/evidence/25-openhab-local-ui-access.png
docs/evidence/26-myopenhab-remote-access-online.png
```

Avoid screenshots that reveal real passwords. If a screenshot contains a password field, crop or obscure it before adding it to the project evidence.
