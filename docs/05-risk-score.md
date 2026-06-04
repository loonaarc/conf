# Risk Score Design

The risk score is the openHAB fusion layer. It combines simple distributed sensor events into one readable safety state.

## Goal

Instead of triggering the alarm from one raw sensor event, openHAB should combine evidence:

```text
door state
+ motion
+ vibration
+ sound context
+ external weather warning
-> risk score and risk level
```

## Items

```text
Number RiskScore "Risk Score [%.0f]"
String RiskLevel "Risk Level [%s]"
String AlarmState "Alarm State [%s]"
```

Optional TinyML items later:

```text
String TinyMLSoundClass "TinyML Sound Class [%s]"
Number TinyMLSoundConfidence "TinyML Sound Confidence [%.2f]"
Number TinyMLInferenceMs "TinyML Inference Time [%.0f ms]"
```

## Current Scoring

| Evidence | Score |
| --- | --- |
| Door/reed open | +2 |
| Motion detected | +1 |
| Vibration detected | +2 |
| Analog sound above calibrated threshold `> 100` | +2 |
| TinyML class `alarm_like` with high confidence | +3 |
| Active GeoSphere warning | +1 |
| GeoSphere warning level `>= 3` | +1 |

## Risk Levels

| Score | Level | Action |
| --- | --- | --- |
| 0-1 | `LOW` | Show only |
| 2-3 | `MEDIUM` | Show warning |
| 4-5 | `HIGH` | Activate local alarm |
| 6+ | `CRITICAL` | Activate alarm and mark escalation placeholder |

## Current Implementation

The first openHAB risk-score version is implemented.

Current behavior:

```text
sensor/context update
  -> Recalculate Safety Risk Score and Alarm State rule
  -> update RiskScore and RiskLevel
  -> Risk Level Triggers Alarm rule activates relay/buzzer when risk is HIGH or CRITICAL
  -> Touch Acknowledges Alarm rule silences relay/buzzer and sets AlarmState to ACKNOWLEDGED
```

The previous direct trigger rules:

```text
door AND motion
door AND vibration
```

are currently commented out, so the main alarm path is now based on calculated risk level.

The analog microphone value is kept in the score for now, but it is still a weak telemetry-based sound signal. The future ESP32 TinyML audio node should become the stronger audio event source by publishing classification labels and confidence values.

## UI Placement

After the sitemap is restructured by physical node, the risk score should get its own small summary frame near the top:

```text
Safety Summary
  RiskScore
  RiskLevel
  AlarmState
```

The node frames should remain focused on node-specific status, sensors, and actuators.
