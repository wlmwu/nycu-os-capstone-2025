#!/bin/bash

tmux new-session -d -s debug_session \
  "tmux set -g mouse on; make run GDB" \; \
  split-window -h -t debug_session "aarch64-elf-gdb" \; \
  attach -t debug_session