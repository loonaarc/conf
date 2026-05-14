# Architecture

This document explains how the configuration is structured and how data moves through the system. For installation, startup commands, and troubleshooting, see [README.md](README.md).

The lab material builds the system in three stages:

1. Lab 1 prepares the D1 Mini with Tasmota, relay hardware, MQTT, and an MQTT client.
2. Lab 3 preparations install openHAB and recommend Visual Studio Code with the openHAB extension.
3. Lab 3 integrates the MQTT device into openHAB using text-based configuration files.

## Components

openHAB does not talk to the Tasmota device directly. The device and openHAB exchange messages through a local Mosquitto MQTT broker.

```text
+-------------------+       MQTT        +-------------------+
| Tasmota D1 Mini   | <---------------> | Mosquitto broker  |
| Relay / switch    |                   | localhost:1883    |
+-------------------+                   +-------------------+
                                                ^
                                                |
                                                | MQTT binding
                                                v
                                      +-------------------+
                                      | openHAB           |
                                      | Things / Items    |
                                      | Rules / Basic UI  |
                                      +-------------------+
```

## openHAB Configuration Flow

```text
things/mqtt.things
  defines MQTT broker and MQTT device channels
        |
        v
items/lab3.items
  links channels to openHAB items
        |
        +--> sitemaps/lab3.sitemap
        |      displays items in Basic UI
        |
        +--> rules/lab3.rules
               reacts to item state changes
```

This follows the Lab 3 requirement that the openHAB configuration is done with files, not through MainUI-managed objects.

## MQTT Thing Layer

`things/mqtt.things` defines two levels:

- `Bridge mqtt:broker:mosquitto` is the connection from openHAB to Mosquitto.
- `Thing mqtt:topic:d1mini` is the logical MQTT device using that broker.

The D1 Mini thing exposes two channels:

- `relay` sends commands to `cmnd/safety_monitor_1/POWER` and reads state from `stat/safety_monitor_1/POWER`.
- `motion` reads JSON messages from `stat/safety_monitor_1/RESULT` and extracts `$.Switch1.Action`.

If the Tasmota device topic changes, the `safety_monitor_1` part must be changed in this file.

Tasmota also publishes general telemetry under:

```text
tele/safety_monitor_1/
```

Examples from Lab 1:

```text
tele/safety_monitor_1/LWT
tele/safety_monitor_1/SENSOR
```

Topics below `$SYS` are broker status topics from Mosquitto itself. They are useful for checking the broker, but they are not sensor or actuator data from the D1 Mini.

In the Lab 3 handout, the same pattern is also used for a DS18B20 temperature sensor by adding another MQTT channel and extracting the temperature from the Tasmota JSON payload with JSONPATH. This local configuration currently uses a motion-style JSON field instead:

```text
$.Switch1.Action
```

## Item Layer

`items/lab3.items` creates openHAB items:

```text
Relay             -> mqtt:topic:d1mini:relay
Motion            -> mqtt:topic:d1mini:motion
MotionAutomation  -> local switch, not linked to MQTT
```

`Relay` and `Motion` are connected to MQTT channels. `MotionAutomation` is only used inside openHAB as an enable/disable switch for the rule.

In the original temperature example from the handout, this layer would also include a number item for the measured temperature and another item for the threshold value.

## Rule Layer

`rules/lab3.rules` contains one rule:

```text
Motion changed to ON
```

When this happens, openHAB checks whether `MotionAutomation` is ON. If it is, the relay is toggled:

- Relay ON becomes OFF.
- Relay OFF becomes ON.

The Lab 3 handout describes an automation rule based on the DS18B20 temperature crossing a threshold. This configuration keeps the same openHAB rule concept but applies it to motion: motion is the trigger, and `MotionAutomation` is the local enable switch.

## UI Layer

`sitemaps/lab3.sitemap` defines the Basic UI page named `lab3`.

It has two frames:

- `Device` shows the relay switch and motion state.
- `Control` shows the automation enable switch.

The handout's Basic UI example starts with a relay switch, then adds sensor and automation controls. This sitemap follows that structure with the currently implemented relay, motion state, and motion automation switch.

## Add-on Layer

`services/addons.cfg` installs the add-ons required by the file-based setup:

```text
binding = mqtt
transformation = jsonpath
ui = basic
```

The MQTT binding connects openHAB to Mosquitto, JSONPATH extracts values from JSON MQTT payloads, and Basic UI renders the sitemap.
