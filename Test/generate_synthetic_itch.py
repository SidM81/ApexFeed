#!/usr/bin/env python3
import argparse
from pathlib import Path


def be6(value: int) -> bytes:
    return value.to_bytes(6, "big")


def add_order(order_id: int, side: str, qty: int, stock: str, price: int, timestamp: int) -> bytes:
    message = bytearray(36)
    message[0:1] = b"A"
    message[1:3] = (1).to_bytes(2, "big")
    message[3:5] = (1).to_bytes(2, "big")
    message[5:11] = be6(timestamp)
    message[11:19] = order_id.to_bytes(8, "big")
    message[19:20] = side.encode("ascii")
    message[20:24] = qty.to_bytes(4, "big")
    message[24:32] = stock.encode("ascii")[:8].ljust(8, b" ")
    message[32:36] = price.to_bytes(4, "big")
    return bytes(message)


def cancel_order(order_id: int, qty: int, timestamp: int) -> bytes:
    message = bytearray(23)
    message[0:1] = b"X"
    message[1:3] = (1).to_bytes(2, "big")
    message[3:5] = (1).to_bytes(2, "big")
    message[5:11] = be6(timestamp)
    message[11:19] = order_id.to_bytes(8, "big")
    message[19:23] = qty.to_bytes(4, "big")
    return bytes(message)


def symbol_name(index: int) -> str:
    return f"S{index:07d}"[:8]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate a deterministic ITCH-style binary feed for ApexAlpha tests.")
    parser.add_argument("--out", required=True, help="Output binary file path.")
    parser.add_argument("--manifest", required=True, help="Output manifest file path.")
    parser.add_argument("--symbols", type=int, default=16, help="Number of symbols to generate.")
    parser.add_argument("--cycles", type=int, default=1000, help="Number of trade cycles per symbol.")
    parser.add_argument("--base-price", type=int, default=10000, help="Starting base price.")
    parser.add_argument("--price-step", type=int, default=25, help="Price offset between symbols.")
    parser.add_argument("--base-qty", type=int, default=1000, help="Base resting size per side.")
    parser.add_argument("--long-boost-qty", type=int, default=4000, help="Extra bid size that triggers long entry.")
    parser.add_argument("--exit-ask-qty", type=int, default=7000, help="Extra ask size that forces long exit.")
    parser.add_argument("--short-boost-qty", type=int, default=3000, help="Extra ask size that triggers short entry.")
    parser.add_argument("--exit-bid-qty", type=int, default=9000, help="Extra bid size that forces short exit.")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    out_path = Path(args.out)
    manifest_path = Path(args.manifest)
    out_path.parent.mkdir(parents=True, exist_ok=True)
    manifest_path.parent.mkdir(parents=True, exist_ok=True)

    next_order_id = 1
    timestamp = 1
    message_count = 0
    total_bytes = 0

    with out_path.open("wb") as output:
        for symbol_index in range(args.symbols):
            symbol = symbol_name(symbol_index)
            base_price = args.base_price + (symbol_index * args.price_step)

            for _ in range(args.cycles):
                bid_base = next_order_id
                ask_base = next_order_id + 1
                bid_long = next_order_id + 2
                ask_exit = next_order_id + 3
                ask_short = next_order_id + 4
                bid_exit = next_order_id + 5
                next_order_id += 6

                messages = (
                    add_order(bid_base, "B", args.base_qty, symbol, base_price, timestamp),
                    add_order(ask_base, "S", args.base_qty, symbol, base_price + 10, timestamp + 1),
                    add_order(bid_long, "B", args.long_boost_qty, symbol, base_price, timestamp + 2),
                    add_order(ask_exit, "S", args.exit_ask_qty, symbol, base_price + 10, timestamp + 3),
                    add_order(ask_short, "S", args.short_boost_qty, symbol, base_price + 10, timestamp + 4),
                    add_order(bid_exit, "B", args.exit_bid_qty, symbol, base_price, timestamp + 5),
                    cancel_order(ask_base, args.base_qty, timestamp + 6),
                    cancel_order(ask_exit, args.exit_ask_qty, timestamp + 7),
                    cancel_order(ask_short, args.short_boost_qty, timestamp + 8),
                    cancel_order(bid_base, args.base_qty, timestamp + 9),
                    cancel_order(bid_long, args.long_boost_qty, timestamp + 10),
                    cancel_order(bid_exit, args.exit_bid_qty, timestamp + 11),
                )

                for message in messages:
                    output.write(message)
                    message_count += 1
                    total_bytes += len(message)

                timestamp += len(messages)

    expected_trade_events = args.symbols * args.cycles * 4
    manifest_lines = [
        f"output={out_path}",
        f"symbols={args.symbols}",
        f"cycles={args.cycles}",
        f"messages={message_count}",
        f"bytes={total_bytes}",
        f"expected_trade_events={expected_trade_events}",
        f"base_qty={args.base_qty}",
        f"long_boost_qty={args.long_boost_qty}",
        f"exit_ask_qty={args.exit_ask_qty}",
        f"short_boost_qty={args.short_boost_qty}",
        f"exit_bid_qty={args.exit_bid_qty}",
    ]
    manifest_path.write_text("\n".join(manifest_lines) + "\n", encoding="ascii")
    print(f"[GEN] output={out_path}")
    print(f"[GEN] manifest={manifest_path}")
    print(f"[GEN] messages={message_count}")
    print(f"[GEN] bytes={total_bytes}")
    print(f"[GEN] expected_trade_events={expected_trade_events}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
