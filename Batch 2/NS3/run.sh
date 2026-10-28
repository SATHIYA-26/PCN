#!/bin/bash
rm -rf results && mkdir -p results
for p in rip ospf aodv; do
  for s in normal congestion failure; do
    ./ns3 run "scratch/dept-routing --proto=$p --scenario=$s --outDir=results"
  done
done
python3 plot_results.py
