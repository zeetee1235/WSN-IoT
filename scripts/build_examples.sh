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

TARGET=cooja

build_example() {
  local path="$1"
  echo "==> build $path"
  (cd "$CONTIKI_DIR/$path" && make TARGET=$TARGET)
}

build_example examples/rpl-udp
build_example examples/coap/coap-example-server
build_example examples/coap/coap-example-client
build_example examples/mqtt/mqtt-client

echo "Build complete."
