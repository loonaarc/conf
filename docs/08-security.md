# Security Notes

This project is a lab prototype. The current setup prioritizes demonstrability and local control, but the security limitations should be documented honestly.

## Current Setup

| Area | Current state |
| --- | --- |
| Network | Local Wi-Fi/hotspot |
| MQTT broker | Mosquitto on the local machine |
| MQTT transport | Plain TCP on port `1883` |
| openHAB UI | Local browser access |
| Tasmota devices | Local web UIs |
| External HTTP input | GeoSphere Austria warning API |

## Current Risks

| Risk | Explanation |
| --- | --- |
| MQTT without TLS | MQTT messages are not encrypted in the lab setup |
| Weak network separation | IoT devices share the local network/hotspot |
| Device web UIs | Tasmota web UIs should not be exposed to untrusted networks |
| openHAB exposure | openHAB should not be directly port-forwarded to the internet |
| MQTT access control | Anonymous MQTT is convenient for testing but weak for real deployments |

## Planned Improvements

| Improvement | Purpose |
| --- | --- |
| MQTT username/password | Prevent casual unauthorized publishing |
| MQTT TLS | Encrypt broker communication |
| Separate IoT network | Reduce impact if one IoT device is compromised |
| Strong Tasmota/openHAB passwords | Protect local device administration |
| VPN for remote access | Avoid direct openHAB exposure |
| Device LWT monitoring | Detect offline or unstable nodes |

## TinyML Privacy Note

The TinyML audio node should publish sound classes and confidence values instead of raw audio:

```text
raw audio stays local
label/confidence goes to MQTT
```

This is one of the strongest privacy arguments for the TinyML extension.
