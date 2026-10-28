import pandas as pd, numpy as np, matplotlib.pyplot as plt, os

df = pd.read_csv("results/results.csv")
protos = ["rip", "ospf", "aodv"]
conds = ["normal", "congestion", "failure"]
clabels = ["Normal", "Congestion", "Link failure"]

def grouped_bar(col, ylabel, title, fname, fmt="%.1f"):
    x, w = np.arange(len(conds)), 0.25
    fig, ax = plt.subplots(figsize=(7.5, 4.8))
    for i, p in enumerate(protos):
        vals = []
        for c in conds:
            r = df[(df.protocol == p) & (df.scenario == c)][col]
            vals.append(r.values[0] if len(r) else np.nan)
        bars = ax.bar(x + (i - 1) * w, vals, w, label=p.upper())
        ax.bar_label(bars, fmt=fmt, fontsize=8, padding=2)
    ax.set_xticks(x); ax.set_xticklabels(clabels)
    ax.set_xlabel("Network condition"); ax.set_ylabel(ylabel); ax.set_title(title)
    ax.grid(axis="y", alpha=0.3); ax.legend()
    fig.tight_layout(); fig.savefig(f"results/{fname}.png", dpi=200); plt.close(fig)

# --- the three required graphs ---
grouped_bar("pdr_pct", "Packet Delivery Ratio (%)", "PDR vs Network Condition", "pdr")
grouped_bar("avg_delay_ms", "Average end-to-end delay (ms)", "End-to-End Delay vs Network Condition", "delay")
grouped_bar("route_updates", "Routing-table updates (entry changes)", "Routing Table Updates vs Network Condition", "route_updates", "%.0f")

# --- extra metrics ---
grouped_bar("throughput_kbps", "Throughput (kbps)", "Throughput vs Network Condition", "throughput")
grouped_bar("avg_jitter_ms", "Jitter (ms)", "Jitter vs Network Condition", "jitter", "%.2f")
grouped_bar("ctrl_bytes", "Routing control bytes", "Control Overhead vs Network Condition", "overhead", "%.0f")
grouped_bar("max_gap_s", "Max service gap of F1 (s)", "Recovery / Outage Time vs Network Condition", "outage", "%.1f")
grouped_bar("route_updates_after_failtime", "Updates after t = failTime", "Routing Updates After t=20 s", "route_updates_after20", "%.0f")

# --- throughput of flow p1->p3 over time (shows failure and recovery) ---
for sc in conds:
    fig, ax = plt.subplots(figsize=(8, 4.2))
    for p in protos:
        f = f"results/ts_{p}_{sc}.csv"
        if os.path.exists(f):
            t = pd.read_csv(f)
            ax.plot(t.time_s, t.f1_throughput_kbps, marker="o", ms=3, label=p.upper())
    if sc == "failure":
        ax.axvspan(20, 40, color="red", alpha=0.12, label="r1-r3 down")
    ax.set_xlabel("Time (s)"); ax.set_ylabel("p1→p3 received throughput (kbps)")
    ax.set_title(f"Flow p1→p3 over time – {sc}")
    ax.grid(alpha=0.3); ax.legend()
    fig.tight_layout(); fig.savefig(f"results/timeseries_{sc}.png", dpi=200); plt.close(fig)

print(df.round(2).to_string(index=False))
