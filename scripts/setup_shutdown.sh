#!/bin/bash
# Setup shutdown commands in rootfs

cd "$(dirname "$0")/../rootfs/sbin"

# Make shutdown executable
chmod +x shutdown

# Create symlinks for convenience
ln -sf shutdown halt 2>/dev/null
ln -sf shutdown reboot 2>/dev/null
ln -sf shutdown poweroff 2>/dev/null

echo "✓ Shutdown commands installed"
ls -la shutdown halt reboot poweroff 2>/dev/null || true
