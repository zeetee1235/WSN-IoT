#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
LOG_DIR="$ROOT_DIR/logs"
mkdir -p "$LOG_DIR"

echo "Cooja GUI에서 로그를 복사해 붙여넣거나,"
echo "Cooja Simulation -> Tools -> Log Listener 내용을 저장하세요."
echo "저장할 파일명을 입력하세요 (예: run1.log):"
read -r fname

echo "이제 터미널에 로그를 붙여넣고 Ctrl+D로 종료:"
cat > "$LOG_DIR/$fname"

echo "Saved: $LOG_DIR/$fname"
