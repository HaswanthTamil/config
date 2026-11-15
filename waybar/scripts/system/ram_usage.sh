#!/usr/bin/env bash
# RAM utilization in percentage and absolute units

MEM_TOTAL=$(grep MemTotal /proc/meminfo | awk '{print $2}')
MEM_AVAIL=$(grep MemAvailable /proc/meminfo | awk '{print $2}')

if [ -n "$MEM_TOTAL" ] && [ -n "$MEM_AVAIL" ] && [ "$MEM_TOTAL" -gt 0 ]; then
    MEM_USED=$(( MEM_TOTAL - MEM_AVAIL ))
    PERCENT=$(( MEM_USED * 100 / MEM_TOTAL ))
    
    # Calculate GB units
    USED_GB=$(awk "BEGIN {printf \"%.1f\", $MEM_USED / 1048576}")
    TOTAL_GB=$(awk "BEGIN {printf \"%.1f\", $MEM_TOTAL / 1048576}")
    
    CLASS="normal"
    if [ "$PERCENT" -ge 90 ]; then
        CLASS="critical"
    elif [ "$PERCENT" -ge 75 ]; then
        CLASS="warning"
    fi

    echo "{\"text\": \"${PERCENT}%\", \"class\": \"$CLASS\", \"tooltip\": \"RAM Utilization: ${PERCENT}%\\nUsed: ${USED_GB} GB / ${TOTAL_GB} GB\\nClick to open Memory Panel\"}"
else
    echo '{"text": "N/A", "class": "unknown", "tooltip": "Memory info unavailable"}'
fi
