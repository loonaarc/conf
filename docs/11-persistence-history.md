# Persistence And Historical Values

The project uses JDBC persistence with SQLite as a non-default persistence backend.

This satisfies two final-check areas:

```text
Persistence implementation, not default rrd4j
Display historical values in openHAB UI
```

## Database

Database file:

```text
userdata/persistence/safety_monitor.db
```

The database is local to the openHAB installation and does not require a separate database server.

## Configuration Files

| File | Purpose |
| --- | --- |
| `services/addons.cfg` | Installs the `jdbc-sqlite` persistence add-on |
| `services/jdbc.cfg` | Configures the SQLite JDBC database URL |
| `services/runtime.cfg` | Sets JDBC as the default persistence service |
| `persistence/jdbc.persist` | Defines which Items are persisted and when |
| `sitemaps/safety_monitor.sitemap` | Displays persisted numeric values as history charts |

## Persisted Items

```text
RiskScore
RiskLevel
AlarmState
Temperature
SoundLevel
Relay
Buzzer
MonitorNodeStatus
AlarmNodeStatus
ContextNodeStatus
```

Numeric Items shown as Basic UI history charts:

```text
RiskScore
Temperature
SoundLevel
```

## Persistence Strategy

Numeric history values are persisted on every change and once per minute:

```text
RiskScore
Temperature
SoundLevel
```

State and status values are persisted on every change:

```text
RiskLevel
AlarmState
Relay
Buzzer
MonitorNodeStatus
AlarmNodeStatus
ContextNodeStatus
```

The selected Items also use `restoreOnStartup` so openHAB can restore the latest known state after a restart.

## TinyML Extension Later

The future ESP32 audio node can use the same persistence backend.

Planned TinyML Items:

```text
TinyMLSoundClass
TinyMLSoundConfidence
TinyMLSoundRms
TinyMLInferenceMs
```

Recommended history charts after TinyML integration:

```text
TinyMLSoundConfidence
TinyMLSoundRms
TinyMLInferenceMs
```

This keeps the TinyML extension aligned with the existing safety-monitoring history instead of creating a separate data path.
