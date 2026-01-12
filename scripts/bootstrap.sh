#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo "[1/4] 기본 툴 체크"
for cmd in git make gcc java javac python3; do
  if ! command -v "$cmd" >/dev/null 2>&1; then
    echo "Missing: $cmd"
  fi
done

echo "[2/4] Contiki-NG 위치 체크"
if [ -d "$ROOT_DIR/contiki-ng" ]; then
  CONTIKI_DIR="$ROOT_DIR/contiki-ng"
elif [ -d "$ROOT_DIR/external/contiki-ng" ]; then
  CONTIKI_DIR="$ROOT_DIR/external/contiki-ng"
else
  echo "Contiki-NG not found. Use clone or symlink first."
  exit 1
fi
echo "CONTIKI_DIR=$CONTIKI_DIR"

echo "[3/4] Cooja 디렉토리 확인"
if [ ! -d "$CONTIKI_DIR/tools/cooja" ]; then
  echo "Cooja directory missing. Check submodules."
  exit 1
fi

echo "[4/4] 완료"
