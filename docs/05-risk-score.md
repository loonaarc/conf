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

## Planned Items

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

## Initial Scoring

| Evidence | Score |
| --- | --- |
| Door/reed open | +2 |
| Motion detected | +1 |
| Vibration detected | +3 |
| Analog sound above threshold | +2 |
| TinyML class `alarm_like` with high confidence | +3 |
| Active GeoSphere warning | +1 |
| Multiple events close together | +2 |

## Risk Levels

| Score | Level | Action |
| --- | --- | --- |
| 0-1 | `LOW` | Show only |
| 2-3 | `MEDIUM` | Show warning |
| 4-5 | `HIGH` | Activate local alarm |
| 6+ | `CRITICAL` | Activate alarm and mark escalation placeholder |

## Implementation Notes

Start with the already implemented behavior:

```text
door AND (motion OR vibration)
  -> alarm ON
```

Then replace the direct alarm trigger with:

```text
sensor update
  -> recalculate RiskScore and RiskLevel
  -> if HIGH or CRITICAL, activate relay and buzzer
```

The TinyML audio node should only add evidence. The safety system should still be demonstrable without the ESP32 audio node.

## UI Placement

After the sitemap is restructured by physical node, the risk score should get its own small summary frame near the top:

```text
Safety Summary
  RiskScore
  RiskLevel
  AlarmState
```

The node frames should remain focused on node-specific status, sensors, and actuators.
