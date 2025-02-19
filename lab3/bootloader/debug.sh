#!/bin/bash

tmux new-session -d -s debug_session \
  "tmux set -g mouse on; make run PTY" \;
echo "Start QEMU..."
sleep 1
qemu_output=$(tmux capture-pane -p -t debug_session)
serial_port=$(echo "$qemu_output" | grep "char device redirected to" | awk '{print $5}')
tmux split-window -v -t debug_session \
  "picocom -b 115200 $serial_port" \; \
  attach -t debug_session