#!/bin/bash

tmux new-session -d -s debug_session \
  "tmux set -g mouse on; make run PTY" \; \
  split-window -v -t debug_session  \; \
  send-key "picocom -b 115200 /dev/ttys" \; \
  attach -t debug_session