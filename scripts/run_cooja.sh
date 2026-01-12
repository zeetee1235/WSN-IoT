#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
if [ -d "$ROOT_DIR/contiki-ng" ]; then
  CONTIKI_DIR="$ROOT_DIR/contiki-ng"
elif [ -d "$ROOT_DIR/external/contiki-ng" ]; then
  CONTIKI_DIR="$ROOT_DIR/external/contiki-ng"
else
  echo "Contiki-NG not found."
  exit 1
fi

COOJA_DIR="$CONTIKI_DIR/tools/cooja"

# Wayland 환경에서 GUI 문제 대비: X11 강제
export GDK_BACKEND=x11
export _JAVA_AWT_WM_NONREPARENTING=1

cd "$COOJA_DIR"
./gradlew run
