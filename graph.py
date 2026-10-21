
import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("tcp_udp_results.csv")

metrics = [
    ("ThroughputMbps", "Throughput", "Mbps"),
    ("PacketLossPercent", "Packet Loss", "%"),
    ("PDRPercent", "Packet Delivery Ratio", "%"),
    ("TransmissionRatioPercent", "Transmission Ratio", "%"),
    ("EndToEndDelayMs", "End-to-End Delay", "ms")
]

for column, title, unit in metrics:
    plt.figure(figsize=(7, 5))

    plt.bar(df["Protocol"], df[column])

    plt.title("TCP vs UDP - " + title)
    plt.xlabel("Protocol")
    plt.ylabel(unit)
    plt.grid(axis="y", linestyle="--", alpha=0.4)

    plt.tight_layout()
    filename = title.lower().replace(" ", "_") + ".png"
    plt.savefig(filename, dpi=150)
    plt.show()

    print("Graph saved:", filename)
