# RIP vs OSPF vs AODV in NS-3 (Normal / Congestion / Link-Failure)

## 1. What this project does

Simulates a department network (3 hosts + 3 routers) for **60 s** and compares **RIP, OSPF (proxy) and AODV** under three conditions:

| Condition | What happens |
|---|---|
| Normal | 3 CBR/UDP flows, light load |
| Congestion | Normal flows + 6 extra simultaneous flows that saturate the 5 Mbps r1-r3 bottleneck |
| Link failure | Link r1-r3 goes down at **20 s**, comes back at **40 s** (20 s outage) |

Total = 3 protocols x 3 conditions = **9 runs**.

---

## 2. Topology

```
                 (backup / bottleneck, 5 Mbps, FAILS)
              +-------------------------------+
              |                               |
 p1 --- r1 ------- r2 ------- r3 --- p3
                    |
                    p2
```

| Link | Subnet | Rate / Delay |
|---|---|---|
| p1-r1 | 10.0.1.0/24 | 100 Mbps / 2 ms |
| r1-r2 | 10.0.2.0/24 | 10 Mbps / 5 ms |
| r2-r3 | 10.0.3.0/24 | 10 Mbps / 5 ms |
| r3-p3 | 10.0.4.0/24 | 100 Mbps / 2 ms |
| r2-p2 | 10.0.5.0/24 | 100 Mbps / 2 ms |
| r1-r3 (added) | 10.0.6.0/24 | 5 Mbps / 5 ms |

**Why r1-r3 was added:** in a pure chain (p1-r1-r2-r3-p3) there is no alternate path, so a failure just disconnects p1 from p3 and no protocol can reroute. r1-r3 is the shortest path (fewest hops), so all protocols use it first. When it fails, traffic moves to r1-r2-r3.

**Traffic**

| Flow | Path | Rate | Port |
|---|---|---|---|
| F1 | p1 to p3 | 1 Mbps | 9001 |
| F2 | p2 to p3 | 1 Mbps | 9002 |
| F3 | p1 to p2 | 0.5 Mbps | 9003 |
| Extra (congestion only) | 4 x p1 to p3 at 1.5 Mbps, 2 x p2 to p3 at 2 Mbps | | 9004-9009 |

All flows: UDP, 1000-byte packets, constant bit rate, running 5 s to 58 s.

---

## 3. Files

| File | Purpose |
|---|---|
| `dept-routing.cc` | The NS-3 simulation (copy into `scratch/`) |
| `run_all.sh` | Runs all 9 combinations, then plots |
| `plot_results.py` | Generates all graphs from the CSVs |

---

## 4. Requirements

- NS-3 **3.36 or later** (uses the `./ns3` launcher), built with default modules
- Python 3 with `pandas`, `numpy`, `matplotlib`

```bash
pip install pandas numpy matplotlib
```

---

## 5. How to run

```bash
# 1. Put the files in place (from your ns-3 root folder, e.g. ns-3.xx/)
cp dept-routing.cc scratch/
cp run_all.sh plot_results.py .

# 2. Run everything (9 simulations + plots)
chmod +x run_all.sh
./run_all.sh
```

**Run a single case manually**

```bash
mkdir -p results
./ns3 run "scratch/dept-routing --proto=rip  --scenario=normal     --outDir=results"
./ns3 run "scratch/dept-routing --proto=ospf --scenario=congestion --outDir=results"
./ns3 run "scratch/dept-routing --proto=aodv --scenario=failure    --outDir=results"
python3 plot_results.py
```

### Command-line options

| Option | Values | Default |
|---|---|---|
| `--proto` | `rip`, `ospf`, `aodv` | `rip` |
| `--scenario` | `normal`, `congestion`, `failure` | `normal` |
| `--simTime` | seconds | 60 |
| `--failTime` | link-down time (s) | 20 |
| `--restoreTime` | link-up time (s); `-1` = never restore | 40 |
| `--outDir` | output folder (must exist) | `results` |

Example, link fails at 15 s and never returns:
```bash
./ns3 run "scratch/dept-routing --proto=aodv --scenario=failure --failTime=15 --restoreTime=-1 --outDir=results"
```

---

## 6. Outputs (in `results/`)

| File | Content |
|---|---|
| `results.csv` | One row per run with all metrics |
| `ts_<proto>_<scenario>.csv` | F1 throughput per second |
| `pdr.png` | **PDR vs Network Condition** (required) |
| `delay.png` | **End-to-End Delay vs Network Condition** (required) |
| `route_updates.png` | **Routing Table Updates vs Network Condition** (required) |
| `throughput.png`, `jitter.png`, `overhead.png`, `outage.png`, `route_updates_after20.png` | Extra metrics |
| `timeseries_normal/congestion/failure.png` | p1 to p3 throughput over time, showing the dip and recovery |

> Delete `results/` (run_all.sh does this) before re-running, otherwise rows are appended to the old CSV.

---

## 7. Where to change things (in `dept-routing.cc`)

| What you want to change | Where |
|---|---|
| Simulation length | `--simTime` or `double simTime = 60.0;` in `main()` |
| Failure / restore time | `--failTime`, `--restoreTime` |
| Traffic start / stop | `double appStart = 5.0, appStop = 58.0;` |
| Link speed / delay | The six `mk(...)` calls, e.g. `mk(r1, r3, "5Mbps", "5ms")` |
| IP subnets | The `a.SetBase("10.0.x.0", ...)` lines |
| Which link fails | The `d6` link and the error models `em1`, `em2` |
| Normal flow rates | The three `AddCbr(...)` calls for F1, F2, F3 |
| Congestion level | The two `for` loops under `if (scenario == "congestion")`: change the loop count (number of flows) or the rate string |
| Packet size | `PacketSize` in `AddCbr()` (1000 bytes) |
| RIP timers | `ripUpdate = 5.0`, `ripTimeout = 15.0` (also GarbageCollectionDelay in the RIP block) |
| AODV timers | `aodvHello = 1.0`, `aodvLoss = 2` |
| OSPF dead / hello interval | `ospfDead = 4.0`, `ospfHello = 1.0` |
| Route-update sampling interval | `Schedule(Seconds(0.1), &SampleTables, ...)` in `SampleTables()` |
| Plot style / which metrics | `plot_results.py`, `grouped_bar(...)` calls |

---

## 8. Metrics: definitions

| Metric | Formula / meaning |
|---|---|
| **PDR** | received packets / sent packets x 100 |
| **End-to-end delay** | mean of (rx time - tx time) over received packets (FlowMonitor `delaySum / rxPackets`) |
| **Routing-table updates** | number of routing-entry additions, removals or changes across all 6 nodes, sampled every 0.1 s |
| Throughput | received bits / application duration |
| Jitter | mean variation in packet delay |
| Control overhead | bytes of RIP (UDP 520) and AODV (UDP 654) packets |
| Max gap | longest silence in F1 reception = outage / reroute time |
| Initial convergence | time from traffic start to first F1 packet |

---

## 9. Expected behaviour (what to say in viva)

- **Normal:** PDR near 100% for all. RIP and OSPF have slightly lower delay than AODV at the start, since AODV must discover a route first (RREQ/RREP).
- **Congestion:** The 5 Mbps r1-r3 link is overloaded, so queues drop packets. PDR falls and delay rises for all protocols. Routing protocols here do not do load balancing, so differences are small.
- **Link failure:**
  - **OSPF (proxy):** reroutes after the dead interval (about 4 s). Fastest recovery.
  - **AODV:** detects loss via missed HELLOs (about 2-3 s), sends RERR, then rediscovers the route.
  - **RIP:** slowest, waits for the route timeout (15 s) unless a triggered update arrives earlier. Largest outage and PDR drop.
- **Overhead:** RIP sends periodic updates always. AODV sends control traffic mainly on demand. The OSPF proxy shows 0 control bytes because it is computed globally.

---

## 10. Likely viva questions

**Q: Why did you add the r1-r3 link?**
Without it there is only one path, so no protocol can recover from a failure. The extra link gives an alternate route.

**Q: Does NS-3 have OSPF?**
Not in mainline. I used `Ipv4GlobalRouting` as a link-state (SPF/Dijkstra) proxy and modelled detection delay with a 4 s dead interval. For real OSPF, an external module is needed.

**Q: Difference between the three protocols?**
- RIP: distance-vector, hop-count metric (max 15), periodic updates, split horizon with poison reverse.
- OSPF: link-state, each router floods link states and runs Dijkstra, fast convergence.
- AODV: reactive/on-demand, builds routes with RREQ/RREP, repairs with RERR, designed for ad-hoc networks.

**Q: How is the link failure implemented?**
A `RateErrorModel` with 100% packet error rate is enabled on both ends of r1-r3 at 20 s. The link silently drops everything, so RIP and AODV detect it through their own timers like in a real failure.

**Q: How is PDR calculated?**
FlowMonitor counts Tx and Rx packets per flow. PDR = sum(Rx) / sum(Tx) x 100 over data flows (ports 9001-9099). Control traffic is excluded.

**Q: How are routing table updates counted?**
Every 0.1 s each node's table is printed and parsed (destination, gateway, interface, flags). Differences from the previous snapshot are counted as updates. Timers and metrics are ignored so only real route changes count.

**Q: Why does RIP recover slowest?**
It only declares a route dead after the timeout (15 s here, 180 s by default), while OSPF and AODV detect failure faster via hello or dead timers.

**Q: What causes the delay increase in congestion?**
Queueing at the 5 Mbps bottleneck. Once the queue is full, packets are dropped, which lowers PDR.

**Q: Why are the timers shortened?**
So reconvergence fits in a 60 s run. Default RIP timers (30 s update / 180 s timeout) would never show recovery.

---

## 11. Limitations (mention in report)

- OSPF is a proxy, not a full implementation (no LSA/hello packets).
- Route-update counts are not strictly comparable across protocols because table formats differ (AODV also stores neighbour routes).
- Results depend on the shortened timers listed above.
- Each run uses one seed; for stronger results, repeat with different `RngRun` values and average.

---

## 12. Troubleshooting

| Problem | Fix |
|---|---|
| `results/results.csv` not found | Create the folder first: `mkdir -p results` |
| Compile error on `SplitHorizon` | Remove that one `rip.Set("SplitHorizon", ...)` line (older versions differ) |
| `./ns3: not found` | You are on an older NS-3; use `./waf --run "dept-routing --proto=rip ..."` |
| Old rows in CSV | Delete `results/` and re-run |
| Plot script missing module | `pip install pandas numpy matplotlib` |
| OSPF PDR is 0 | Make sure the `PopulateRoutingTables` schedule line is present |
