# 개발환경 세팅안 (Fedora 42, Contiki-NG + Cooja, 코드 중심)

요구사항: Python/Rust 중심, C 수정 최소, GUI 최소, 빌드/실행/로그는 CLI 자동화.

## [0] 전체 디렉토리 구조 제안 (Tree)

```text
WSN-IoT/
├─ contiki-ng/                 # A 케이스: 루트 내부 clone
├─ external/
│  └─ contiki-ng -> ~/contiki-ng  # B 케이스: 심볼릭 링크(선택)
├─ scripts/
│  ├─ bootstrap.sh
│  ├─ build_examples.sh
│  ├─ run_cooja.sh
│  └─ collect_logs.sh
├─ tools/
│  ├─ python/
│  │  └─ log_parser.py
│  └─ rust/
│     ├─ Cargo.toml
│     └─ logstats/
│        ├─ Cargo.toml
│        └─ src/
│           └─ main.rs
├─ logs/
│  └─ .gitkeep
├─ requirements.txt
├─ .editorconfig
└─ memo.md
```

---

## [1] Fedora 패키지 점검/설치 커맨드

점검:

```bash
java -version
javac -version
gradle -v
ant -version
gcc --version
make --version
python3 --version
rustc --version
cargo --version
```

설치(누락 시):

```bash
sudo dnf install -y \
  git make gcc gcc-c++ clang \
  python3 python3-venv python3-pip \
  java-17-openjdk java-17-openjdk-devel \
  ant gradle \
  glib2 glib2-devel \
  libX11 libXext libXrender libXtst libXi \
  mesa-libGL mesa-libEGL \
  ncurses ncurses-devel

# 선택: 그래프/시각화
sudo dnf install -y python3-matplotlib
```

실패 포인트 대비:
- Java/Gradle/Ant: Cooja 빌드 실패 주요 원인
- Wayland: GUI 실행 시 X11 강제 옵션 필요 가능

---

## [2] Contiki-NG 배치(A/B) + cooja 서브모듈/gradle 체크

A) 이 디렉토리 안에 `contiki-ng/`로 clone

```bash
cd /home/dev/WSN-IoT
git clone https://github.com/contiki-ng/contiki-ng.git contiki-ng
cd contiki-ng
git submodule update --init --recursive
```

B) 이미 `~/contiki-ng`가 있을 때(심볼릭 링크 또는 submodule)

```bash
# 심볼릭 링크 방식(권장)
cd /home/dev/WSN-IoT
mkdir -p external
ln -s ~/contiki-ng external/contiki-ng

# 또는 submodule 연결
cd /home/dev/WSN-IoT
git submodule add ~/contiki-ng contiki-ng
git submodule update --init --recursive
```

Cooja 서브모듈/Gradle 체크

```bash
ls contiki-ng/tools/cooja || ls external/contiki-ng/tools/cooja

cd contiki-ng/tools/cooja
./gradlew --version || gradle --version
```

---

## [3] 스크립트 제공

`scripts/bootstrap.sh`

```bash
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
```

`scripts/build_examples.sh`

```bash
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
```

`scripts/run_cooja.sh`

```bash
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
```

`scripts/collect_logs.sh`

```bash
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
```

---

## [4] Python 환경

`requirements.txt`

```text
pandas
numpy
matplotlib
tqdm
```

.venv 생성/활성화:

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

`tools/python/log_parser.py`

```python
#!/usr/bin/env python3
import argparse
import re
import pandas as pd

# 간단한 패턴 예시: "TX node=1 seq=10 ts=123.456"
TX_RE = re.compile(r".*TX.*node=(\d+).*seq=(\d+).*ts=([\d\.]+)")
RX_RE = re.compile(r".*RX.*node=(\d+).*seq=(\d+).*ts=([\d\.]+)")


def parse_lines(lines):
    tx = []
    rx = []
    for line in lines:
        m = TX_RE.match(line)
        if m:
            tx.append((int(m.group(1)), int(m.group(2)), float(m.group(3))))
            continue
        m = RX_RE.match(line)
        if m:
            rx.append((int(m.group(1)), int(m.group(2)), float(m.group(3))))
    return tx, rx


def compute_stats(tx, rx):
    tx_df = pd.DataFrame(tx, columns=["node", "seq", "ts"])
    rx_df = pd.DataFrame(rx, columns=["node", "seq", "ts"])

    # 기본 손실률(단순: TX 대비 RX 매칭)
    merged = pd.merge(tx_df, rx_df, on=["seq"], suffixes=("_tx", "_rx"))
    merged["delay"] = merged["ts_rx"] - merged["ts_tx"]

    loss_rate = 1.0
    if len(tx_df) > 0:
        loss_rate = 1.0 - (len(merged) / len(tx_df))

    return {
        "tx_count": len(tx_df),
        "rx_count": len(rx_df),
        "matched": len(merged),
        "loss_rate": loss_rate,
        "avg_delay": merged["delay"].mean() if len(merged) > 0 else None,
    }


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("logfile")
    args = ap.parse_args()

    with open(args.logfile, "r", encoding="utf-8") as f:
        lines = f.readlines()

    tx, rx = parse_lines(lines)
    stats = compute_stats(tx, rx)

    print(stats)


if __name__ == "__main__":
    main()
```

---

## [5] Rust 환경

`tools/rust/Cargo.toml`

```toml
[workspace]
members = ["logstats"]
resolver = "2"
```

`tools/rust/logstats/Cargo.toml`

```toml
[package]
name = "logstats"
version = "0.1.0"
edition = "2021"

[dependencies]
regex = "1"
clap = { version = "4", features = ["derive"] }
```

`tools/rust/logstats/src/main.rs`

```rust
use clap::Parser;
use regex::Regex;
use std::fs::read_to_string;

#[derive(Parser)]
struct Args {
    logfile: String,
}

fn main() {
    let args = Args::parse();
    let content = read_to_string(&args.logfile).expect("read logfile");
    let tx_re = Regex::new(r"TX.*node=(\d+).*seq=(\d+).*ts=([\d\.]+)").unwrap();
    let rx_re = Regex::new(r"RX.*node=(\d+).*seq=(\d+).*ts=([\d\.]+)").unwrap();

    let mut tx_count = 0u32;
    let mut rx_count = 0u32;

    for line in content.lines() {
        if tx_re.is_match(line) {
            tx_count += 1;
        }
        if rx_re.is_match(line) {
            rx_count += 1;
        }
    }

    println!("tx_count={}", tx_count);
    println!("rx_count={}", rx_count);
}
```

---

## [6] 첫 성공 체크 (5분 절차)

1) 컨티키 위치 확인 및 빌드

```bash
./scripts/bootstrap.sh
./scripts/build_examples.sh
```

2) Cooja 실행

```bash
./scripts/run_cooja.sh
```

3) 예제 시뮬레이션
- Cooja에서 `examples/rpl-udp` 또는 `coap-example-*` 로 시뮬레이션 구성
- Log Listener에서 TX/RX 로그 확인
- `scripts/collect_logs.sh`로 로그 저장

4) 로그 파싱

```bash
source .venv/bin/activate
python tools/python/log_parser.py logs/run1.log
```

---

## 추가 대비 사항

- Java/Gradle/Ant 미설치 시 Cooja 빌드 실패 가능
- Cooja 서브모듈 비어있음: `git submodule update --init --recursive`
- Wayland GUI 문제: `scripts/run_cooja.sh`의 X11 강제 옵션 사용
