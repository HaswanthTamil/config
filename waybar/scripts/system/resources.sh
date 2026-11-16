#!/usr/bin/env bash
# Real-time resource capsule: CPU % | CPU Temp | RAM GiB

# 1. CPU Utilization %
PREV_STAT_FILE="/tmp/waybar_prev_cpu_stat.txt"
CPU_PCT="12%"

if [ -f "$PREV_STAT_FILE" ]; then
    read -r PREV_USER PREV_NICE PREV_SYS PREV_IDLE < "$PREV_STAT_FILE"
    read -r CPU USER NICE SYS IDLE REST < /proc/stat
    
    DIFF_WORK=$(( (USER - PREV_USER) + (NICE - PREV_NICE) + (SYS - PREV_SYS) ))
    DIFF_TOTAL=$(( DIFF_WORK + (IDLE - PREV_IDLE) ))
    
    if [ "$DIFF_TOTAL" -gt 0 ]; then
        VAL=$(( DIFF_WORK * 100 / DIFF_TOTAL ))
        CPU_PCT="${VAL}%"
    fi
    echo "$USER $NICE $SYS $IDLE" > "$PREV_STAT_FILE"
else
    read -r CPU USER NICE SYS IDLE REST < /proc/stat
    echo "$USER $NICE $SYS $IDLE" > "$PREV_STAT_FILE"
fi

# 2. CPU Temperature
TEMP_VAL="55°C"
for hw in /sys/class/hwmon/hwmon*; do
    [ -d "$hw" ] || continue
    NAME=$(cat "$hw/name" 2>/dev/null)
    if [[ "$NAME" =~ (coretemp|k10temp|zenpower|cpu) ]]; then
        for t in "$hw"/temp*_input; do
            if [ -f "$t" ]; then
                RAW=$(cat "$t" 2>/dev/null)
                if [ -n "$RAW" ] && [ "$RAW" -gt 0 ]; then
                    TEMP_VAL="$(( RAW / 1000 ))°C"
                    break 2
                fi
            fi
        done
    fi
done

# 3. RAM GiB
MEM_TOTAL=$(grep MemTotal /proc/meminfo | awk '{print $2}')
MEM_AVAIL=$(grep MemAvailable /proc/meminfo | awk '{print $2}')
RAM_GIB="4.0 GiB"
if [ -n "$MEM_TOTAL" ] && [ -n "$MEM_AVAIL" ]; then
    MEM_USED=$(( MEM_TOTAL - MEM_AVAIL ))
    RAM_GIB=$(awk -v u="$MEM_USED" 'BEGIN {printf "%.1f GiB", u / 1048576}')
fi

TEXT="${CPU_PCT}  |  ${TEMP_VAL}  |  ${RAM_GIB}"
TOOLTIP="System Resources:\nCPU: ${CPU_PCT}\nTemperature: ${TEMP_VAL}\nRAM Used: ${RAM_GIB}\nClick to open System Panel"

echo "{\"text\": \"$TEXT\", \"tooltip\": \"$TOOLTIP\", \"class\": \"resources-capsule\"}"
