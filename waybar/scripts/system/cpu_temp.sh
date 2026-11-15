#!/usr/bin/env bash
# Robust CPU Temperature reader with graceful fallback and sensor identification

TEMP_VAL=""
SENSOR_NAME="Generic Thermal"

# 1. Search hwmon for coretemp, k10temp, zenpower, or cpu
for hw in /sys/class/hwmon/hwmon*; do
    [ -d "$hw" ] || continue
    NAME=$(cat "$hw/name" 2>/dev/null)
    if [[ "$NAME" =~ (coretemp|k10temp|zenpower|cpu) ]]; then
        for t in "$hw"/temp*_input; do
            if [ -f "$t" ]; then
                RAW=$(cat "$t" 2>/dev/null)
                if [ -n "$RAW" ] && [ "$RAW" -gt 0 ]; then
                    TEMP_VAL=$(( RAW / 1000 ))
                    LABEL=$(cat "${t%_input}_label" 2>/dev/null)
                    SENSOR_NAME="$NAME ${LABEL:-Package}"
                    break 2
                fi
            fi
        done
    fi
done

# 2. Fallback to any hwmon temp*_input if still empty
if [ -z "$TEMP_VAL" ]; then
    for t in /sys/class/hwmon/hwmon*/temp*_input; do
        if [ -f "$t" ]; then
            RAW=$(cat "$t" 2>/dev/null)
            if [ -n "$RAW" ] && [ "$RAW" -gt 0 ]; then
                TEMP_VAL=$(( RAW / 1000 ))
                HW_DIR=$(dirname "$t")
                NAME=$(cat "$HW_DIR/name" 2>/dev/null)
                SENSOR_NAME="${NAME:-hwmon} $(basename "$t")"
                break
            fi
        fi
    done
fi

# 3. Fallback to thermal_zone
if [ -z "$TEMP_VAL" ]; then
    for tz in /sys/class/thermal/thermal_zone*; do
        if [ -f "$tz/temp" ]; then
            RAW=$(cat "$tz/temp" 2>/dev/null)
            if [ -n "$RAW" ] && [ "$RAW" -gt 0 ]; then
                TEMP_VAL=$(( RAW / 1000 ))
                TYPE=$(cat "$tz/type" 2>/dev/null)
                SENSOR_NAME="${TYPE:-thermal_zone}"
                break
            fi
        fi
    done
fi

# Fallback if no sensor found
if [ -z "$TEMP_VAL" ]; then
    echo '{"text": "N/A", "class": "unknown", "tooltip": "CPU Temperature sensor unavailable"}'
    exit 0
fi

# Determine temperature class for styling
CLASS="normal"
if [ "$TEMP_VAL" -ge 80 ]; then
    CLASS="critical"
elif [ "$TEMP_VAL" -ge 68 ]; then
    CLASS="warning"
fi

echo "{\"text\": \"${TEMP_VAL}°C\", \"class\": \"$CLASS\", \"tooltip\": \"CPU Temperature: ${TEMP_VAL}°C\\nSource: $SENSOR_NAME\\nClick to open System Panel\"}"
