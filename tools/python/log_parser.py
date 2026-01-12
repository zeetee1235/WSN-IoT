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
