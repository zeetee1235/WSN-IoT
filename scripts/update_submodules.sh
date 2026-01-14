#!/usr/bin/env bash
#
# update_submodules.sh
# Git 서브모듈을 초기화하고 최신 버전으로 업데이트합니다.
#

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

echo "======================================"
echo "서브모듈 업데이트"
echo "======================================"
echo ""

cd "$ROOT_DIR"

# 서브모듈 초기화 (아직 초기화되지 않은 경우)
echo "[1/3] 서브모듈 초기화 중..."
if ! git submodule status | grep -q "^-"; then
    echo "✓ 서브모듈이 이미 초기화되어 있습니다."
else
    git submodule init
    echo "✓ 서브모듈 초기화 완료"
fi
echo ""

# 서브모듈 업데이트
echo "[2/3] 서브모듈 업데이트 중..."
git submodule update --remote --merge
echo "✓ 서브모듈 업데이트 완료"
echo ""

# 서브모듈 상태 확인
echo "[3/3] 서브모듈 상태:"
git submodule status
echo ""

# 각 서브모듈의 브랜치 정보 출력
echo "서브모듈 상세 정보:"
echo "---"
for submodule in $(git config --file .gitmodules --get-regexp path | awk '{print $2}'); do
    if [ -d "$submodule" ]; then
        echo "📦 $submodule"
        cd "$submodule"
        echo "   Branch: $(git branch --show-current)"
        echo "   Commit: $(git log -1 --format='%h - %s (%cr)')"
        echo "   Remote: $(git remote get-url origin 2>/dev/null || echo 'N/A')"
        cd "$ROOT_DIR"
        echo ""
    fi
done

echo "======================================"
echo "✅ 서브모듈 업데이트 완료!"
echo "======================================"
