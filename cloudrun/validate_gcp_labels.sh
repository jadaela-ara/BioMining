#!/bin/bash
# Validate GCP label format to avoid deployment errors

echo "🔍 === GCP LABEL VALIDATION ==="
echo "=============================="

# Test labels from deploy script
LABELS="app=biomining,version=1-0-0,environment=production"

echo "Labels to validate: $LABELS"
echo ""

# Parse and validate each label
IFS=',' read -ra LABEL_ARRAY <<< "$LABELS"

ALL_VALID=true

for label in "${LABEL_ARRAY[@]}"; do
    # Split key=value
    IFS='=' read -r key value <<< "$label"
    
    echo "Testing: $key = $value"
    
    # Validate key format (lowercase letters, numbers, underscores, hyphens, max 63 chars)
    if [[ ! $key =~ ^[a-z0-9_-]{1,63}$ ]]; then
        echo "❌ Invalid key format: $key"
        ALL_VALID=false
    else
        echo "✅ Valid key: $key"
    fi
    
    # Validate value format (lowercase letters, numbers, underscores, hyphens, max 63 chars)
    if [[ ! $value =~ ^[a-z0-9_-]{1,63}$ ]]; then
        echo "❌ Invalid value format: $value"
        ALL_VALID=false
    else
        echo "✅ Valid value: $value"
    fi
    
    echo ""
done

if $ALL_VALID; then
    echo "🎉 All labels are VALID for GCP!"
    exit 0
else
    echo "💥 Some labels are INVALID for GCP!"
    exit 1
fi