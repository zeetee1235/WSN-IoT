# WSN-IoT 개발 환경

Fedora 42 기반 Contiki-NG + Cooja 시뮬레이션 환경

## 디렉토리 구조

```
WSN-IoT/
├── external/
│   └── contiki-ng -> ~/contiki-ng    # Contiki-NG 심볼릭 링크
├── rpl-benchmark/                     # RPL 벤치마크 (서브모듈)
│   ├── receiver_root.c                # RPL root + UDP 수신기
│   ├── sender.c                       # UDP 센서 송신기
│   ├── brpl-of.c                      # BRPL objective function
│   └── run_experiment.sh              # 실험 실행 스크립트
├── scripts/                           # 자동화 스크립트
│   ├── bootstrap.sh                   # 환경 체크
│   ├── build_examples.sh              # 예제 빌드
│   ├── run_cooja.sh                   # Cooja 실행
│   └── collect_logs.sh                # 로그 수집
├── tools/
│   ├── python/
│   │   └── log_parser.py              # Python 로그 분석기
│   └── rust/
│       └── logstats/                  # Rust 로그 분석기
├── logs/                              # 시뮬레이션 로그 저장
└── requirements.txt                   # Python 의존성
```

## 설치된 도구

- ✅ Java 21 (OpenJDK)
- ✅ Ant
- ✅ GCC 15
- ✅ GNU Make
- ✅ Python 3.13
- ✅ Rust 1.92

## 사용법

### 0. 저장소 클론 (서브모듈 포함)

```bash
git clone --recurse-submodules https://github.com/zeetee1235/WSN-IoT.git
cd WSN-IoT
```

기존 클론에 서브모듈 초기화:
```bash
git submodule update --init --recursive
```

또는 자동화 스크립트 사용:
```bash
./scripts/update_submodules.sh
```

### 1. 환경 확인

```bash
./scripts/bootstrap.sh
```

### 2. 예제 빌드

```bash
./scripts/build_examples.sh
```

이 스크립트는 다음 예제들을 빌드합니다:
- `examples/rpl-udp`
- `examples/coap/coap-example-server`
- `examples/coap/coap-example-client`
- `examples/mqtt/mqtt-client`

### 3. Cooja 실행

```bash
./scripts/run_cooja.sh
```

Cooja GUI가 실행됩니다. 시뮬레이션을 구성하고 실행할 수 있습니다.

### 4. 로그 수집

```bash
./scripts/collect_logs.sh
```

Cooja의 로그를 복사하여 `logs/` 디렉토리에 저장합니다.

### 5. 로그 분석

#### Python 사용:

```bash
source .venv/bin/activate.fish  # fish shell
# 또는
source .venv/bin/activate       # bash shell

python tools/python/log_parser.py logs/run1.log
```

#### Rust 사용:

```bash
cd tools/rust
cargo run --release -- ../../logs/run1.log
```

## Python 가상환경

가상환경이 이미 설정되어 있습니다:

```bash
source .venv/bin/activate.fish  # fish shell 사용 시
# 또는
source .venv/bin/activate       # bash shell 사용 시
```

설치된 패키지:
- pandas
- numpy
- matplotlib
- tqdm

## Rust 빌드

Rust 도구는 이미 빌드되어 있습니다:

```bash
cd tools/rust
cargo build --release
```

실행 파일: `tools/rust/target/release/logstats`

## RPL 벤치마크 (서브모듈)

`rpl-benchmark/` 디렉토리는 별도의 저장소로 관리됩니다.
- Repository: https://github.com/zeetee1235/RPL-benchmark.git

### RPL 벤치마크 사용법

```bash
cd rpl-benchmark

# RPL-lite 모드로 3개 센서 실험
./run_experiment.sh rpl-lite 3

# BRPL 모드로 5개 센서 실험
./run_experiment.sh brpl 5

# 로그 확인
ls -lht logs/
```

자세한 사용법은 `rpl-benchmark/readme.md` 참고

## 문제 해결

### Cooja GUI 실행 문제 (Wayland)

`run_cooja.sh` 스크립트에 X11 강제 옵션이 포함되어 있습니다:
- `GDK_BACKEND=x11`
- `_JAVA_AWT_WM_NONREPARENTING=1`

### Cooja 서브모듈 누락

```bash
cd ~/contiki-ng
git submodule update --init --recursive
```

## 다음 단계

1. Cooja에서 시뮬레이션 구성 파일(.csc) 생성
2. 노드 구성 및 시뮬레이션 실행
3. Log Listener로 TX/RX 로그 수집
4. Python 또는 Rust 도구로 로그 분석
5. 결과 시각화 (matplotlib 사용)

## 참고

- Contiki-NG: https://github.com/contiki-ng/contiki-ng
- Cooja 문서: https://docs.contiki-ng.org/en/develop/doc/tutorials/Running-Contiki-NG-in-Cooja.html
