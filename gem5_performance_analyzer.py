#!/usr/bin/env python3
"""
CPU Performance Analysis Report Generator (gem5 O3)
Parses stats.txt and produces a structured HTML report.
"""

from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List, Tuple
import json


@dataclass
class StatEntry:
    raw_value: str
    value: float
    comment: str


class StatsParser:
    def __init__(self, stats_file: str):
        self.stats_file = stats_file
        self.stats: Dict[str, StatEntry] = {}
        self.parse()

    def parse(self) -> None:
        with open(self.stats_file, "r") as f:
            for line in f:
                line = line.strip()
                if not line or line.startswith("-"):
                    continue
                parts = line.split("#", 1)
                left = parts[0].strip()
                comment = parts[1].strip() if len(parts) > 1 else ""
                tokens = left.split()
                if len(tokens) < 2:
                    continue
                key = tokens[0]
                raw_value = tokens[1]
                try:
                    value = float(raw_value)
                except ValueError:
                    continue
                self.stats[key] = StatEntry(raw_value=raw_value, value=value, comment=comment)

    def get_value(self, key: str, default: float = 0.0) -> float:
        entry = self.stats.get(key)
        return entry.value if entry else default

    def get_raw(self, key: str, default: str = "N/A") -> str:
        entry = self.stats.get(key)
        return entry.raw_value if entry else default

    def keys(self) -> List[str]:
        return list(self.stats.keys())


class ReportBuilder:
    def __init__(self, parser: StatsParser):
        self.parser = parser

    def build(self) -> str:
        metrics = self._compute_metrics()
        grouped_stats = self._group_stats()
        charts = self._chart_data(metrics)
        cache_summary = self._cache_summary()
        branch_summary = self._branch_summary()

        return f"""<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>CPU Performance Analysis Report (gem5 O3)</title>
  <style>
    body {{ background: #ffffff; color: #111827; font-family: Arial, sans-serif; margin: 24px; }}
    h1 {{ font-size: 24px; margin-bottom: 12px; }}
    h2 {{ font-size: 18px; margin: 18px 0 10px; }}
    .card {{ border: 1px solid #e5e7eb; border-radius: 8px; padding: 16px; margin-bottom: 16px; background: #f9fafb; }}
    .highlight {{ background: #eef2ff; border-color: #c7d2fe; }}
    table {{ width: 100%; border-collapse: collapse; margin-top: 8px; }}
    th, td {{ border: 1px solid #e5e7eb; padding: 6px 8px; text-align: left; font-size: 13px; }}
    th {{ background: #f3f4f6; }}
    .mono {{ font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, "Liberation Mono", "Courier New", monospace; }}
    .grid {{ display: grid; grid-template-columns: repeat(auto-fit, minmax(260px, 1fr)); gap: 12px; }}
    .chart-grid {{ display: grid; grid-template-columns: repeat(auto-fit, minmax(280px, 1fr)); gap: 16px; }}
    .chart-box {{ background: #ffffff; border: 1px solid #e5e7eb; border-radius: 8px; padding: 8px; height: 260px; }}
    .chart-box canvas {{ width: 100% !important; height: 100% !important; }}
    details {{ margin-bottom: 10px; }}
    summary {{ font-weight: bold; cursor: pointer; }}
  </style>
</head>
<body>
  <h1>CPU Performance Analysis Report (gem5 O3)</h1>

  <div class="card highlight">
    <h2>Executive Summary</h2>
    <table>
      <tr><th>Metric</th><th>Value</th><th>Formula / Raw Values</th></tr>
      <tr>
        <td>Total Cycles</td>
        <td class="mono">{metrics['total_cycles']}</td>
        <td class="mono">{metrics['total_cycles_formula']}</td>
      </tr>
      <tr>
        <td>Commit Cycles (Useful Work)</td>
        <td class="mono">{metrics['commit_cycles']}</td>
        <td class="mono">{metrics['commit_cycles_formula']}</td>
      </tr>
      <tr>
        <td>Stall Cycles (Wasted)</td>
        <td class="mono">{metrics['stall_cycles']}</td>
        <td class="mono">{metrics['stall_cycles_formula']}</td>
      </tr>
      <tr>
        <td>IPC</td>
        <td class="mono">{metrics['ipc']}</td>
        <td class="mono">{metrics['ipc_formula']}</td>
      </tr>
      <tr>
        <td>CPI</td>
        <td class="mono">{metrics['cpi']}</td>
        <td class="mono">{metrics['cpi_formula']}</td>
      </tr>
      <tr>
        <td>Efficiency</td>
        <td class="mono">{metrics['efficiency']}</td>
        <td class="mono">{metrics['efficiency_formula']}</td>
      </tr>
      <tr>
        <td>Avg Commits per Cycle</td>
        <td class="mono">{metrics['avg_commits_per_cycle']}</td>
        <td class="mono">{metrics['avg_commits_per_cycle_formula']}</td>
      </tr>
    </table>
  </div>

  <div class="card">
    <h2>Charts</h2>
    <div class="chart-grid">
      <div class="chart-box">
        <canvas id="commitStallChart"></canvas>
      </div>
      {charts['stall_chart_html']}
      {charts['cache_chart_html']}
      {charts['branch_chart_html']}
    </div>
  </div>

  <div class="card">
    <h2>Bottleneck Analysis</h2>
    <table>
      <tr><th>Type</th><th>Root Cause</th><th>Supporting Metrics</th></tr>
      <tr>
        <td class="mono">{metrics['bottleneck_type']}</td>
        <td>{metrics['bottleneck_cause']}</td>
        <td class="mono">{metrics['bottleneck_support']}</td>
      </tr>
    </table>
  </div>

  <div class="card">
    <h2>Pipeline Stage Analysis</h2>
    <table>
      <tr><th>Stage</th><th>Running</th><th>Stall/Blocked</th><th>Key Issues</th></tr>
      <tr><td>Fetch</td><td class="mono">{metrics['fetch_running']}</td><td class="mono">{metrics['fetch_stall']}</td><td>{metrics['fetch_issue']}</td></tr>
      <tr><td>Decode</td><td class="mono">{metrics['decode_running']}</td><td class="mono">{metrics['decode_stall']}</td><td>{metrics['decode_issue']}</td></tr>
      <tr><td>Rename</td><td class="mono">{metrics['rename_running']}</td><td class="mono">{metrics['rename_stall']}</td><td>{metrics['rename_issue']}</td></tr>
      <tr><td>Execute</td><td class="mono">{metrics['execute_running']}</td><td class="mono">{metrics['execute_stall']}</td><td>{metrics['execute_issue']}</td></tr>
      <tr><td>Commit</td><td class="mono">{metrics['commit_running']}</td><td class="mono">{metrics['commit_stall']}</td><td>{metrics['commit_issue']}</td></tr>
    </table>
  </div>

  <div class="card">
    <h2>Cache Summary</h2>
    <table>
      <tr><th>Cache</th><th>Hits</th><th>Misses</th><th>Accesses</th><th>Miss Rate</th></tr>
      <tr>
        <td>L1D (demand)</td>
        <td class="mono">{cache_summary['l1d_hits']}</td>
        <td class="mono">{cache_summary['l1d_misses']}</td>
        <td class="mono">{cache_summary['l1d_accesses']}</td>
        <td class="mono">{cache_summary['l1d_miss_rate']}</td>
      </tr>
      <tr>
        <td>L1I (demand)</td>
        <td class="mono">{cache_summary['l1i_hits']}</td>
        <td class="mono">{cache_summary['l1i_misses']}</td>
        <td class="mono">{cache_summary['l1i_accesses']}</td>
        <td class="mono">{cache_summary['l1i_miss_rate']}</td>
      </tr>
      <tr>
        <td>L2 (demand)</td>
        <td class="mono">{cache_summary['l2_hits']}</td>
        <td class="mono">{cache_summary['l2_misses']}</td>
        <td class="mono">{cache_summary['l2_accesses']}</td>
        <td class="mono">{cache_summary['l2_miss_rate']}</td>
      </tr>
    </table>
  </div>

  <div class="card">
    <h2>Branch Prediction Summary</h2>
    <table>
      <tr><th>Metric</th><th>Value</th><th>Source</th></tr>
      <tr>
        <td>Status</td>
        <td class="mono">{branch_summary['status']}</td>
        <td>{branch_summary['status_source']}</td>
      </tr>
      <tr>
        <td>Total Branches</td>
        <td class="mono">{branch_summary['total_branches']}</td>
        <td class="mono">{branch_summary['total_branches_source']}</td>
      </tr>
      <tr>
        <td>Mispredictions</td>
        <td class="mono">{branch_summary['mispredictions']}</td>
        <td class="mono">{branch_summary['mispredictions_source']}</td>
      </tr>
      <tr>
        <td>Misprediction Rate</td>
        <td class="mono">{branch_summary['mispred_rate']}</td>
        <td class="mono">{branch_summary['mispred_rate_source']}</td>
      </tr>
    </table>
  </div>

  <div class="card">
    <h2>Detailed Statistics (Full)</h2>
    {self._build_detailed_sections(grouped_stats)}
  </div>

  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <script>
    const commitStallCtx = document.getElementById('commitStallChart').getContext('2d');
    new Chart(commitStallCtx, {{
      type: 'pie',
      data: {{
        labels: ['Commit', 'Stall'],
        datasets: [{{
          data: [{charts['commit_cycles_value']}, {charts['stall_cycles_value']}],
          backgroundColor: ['#4f46e5', '#9ca3af']
        }}]
      }},
      options: {{
        responsive: true,
        maintainAspectRatio: false,
        plugins: {{ legend: {{ position: 'bottom' }} }}
      }}
    }});

    {charts['stall_chart_script']}
    {charts['cache_chart_script']}
    {charts['branch_chart_script']}
  </script>
</body>
</html>
"""

    def _compute_metrics(self) -> Dict[str, str]:
        p = self.parser
        sim_ticks = p.get_value("simTicks")
        sim_seconds = p.get_value("simSeconds")
        sim_insts = p.get_value("simInsts")
        sim_ops = p.get_value("simOps")

        core_cycles = p.get_value("board.processor.cores.core.numCycles")
        total_cycles = sim_ticks
        clock_period = p.get_value("board.clk_domain.clock")
        if clock_period <= 0 and core_cycles > 0 and sim_ticks > 0:
            clock_period = sim_ticks / core_cycles

        ipc_stat = p.get_value("board.processor.cores.core.ipc")
        cpi_stat = p.get_value("board.processor.cores.core.cpi")

        ipc = ipc_stat if ipc_stat > 0 else (sim_insts / total_cycles if total_cycles > 0 else 0.0)
        cpi = cpi_stat if cpi_stat > 0 else (total_cycles / sim_insts if sim_insts > 0 else 0.0)

        commit_hist = self._commit_histogram()
        if commit_hist["total_cycles"] > 0 and clock_period > 0:
            commit_cycles = int(commit_hist["active_cycles"] * clock_period)
            stall_cycles = int(total_cycles - commit_cycles) if total_cycles > 0 else 0
            efficiency_pct = (commit_cycles / total_cycles) * 100.0 if total_cycles > 0 else 0.0
            commit_cycles_formula = (
                "Commit cycles = active_commit_cycles x clock_period = "
                f"{commit_hist['active_cycles']:,} x {clock_period:.6f}"
            )
            efficiency_formula = (
                f"Efficiency = commit_cycles / total_cycles = {commit_cycles:,} / {int(total_cycles):,}"
            )
            avg_commits_per_cycle = commit_hist["avg_commits"]
            avg_commits_formula = (
                "Avg commits/cycle = sum(n*count) / samples from commit.numCommittedDist"
            )
        else:
            commit_width = int(p.get_value("board.processor.cores.core.commit.numCommittedDist::max_value"))
            issue_width = int(p.get_value("board.processor.cores.core.numIssuedDist::max_value"))
            assumed_width = False
            if commit_width <= 0:
                if issue_width > 0:
                    commit_width = issue_width
                else:
                    commit_width = 4
                    assumed_width = True

            commit_eff = ipc / commit_width if commit_width > 0 else 0.0
            if commit_eff > 1.0:
                commit_eff = 1.0

            commit_cycles = int(total_cycles * commit_eff) if commit_eff > 0 else 0
            stall_cycles = int(total_cycles - commit_cycles) if total_cycles > 0 else 0

            efficiency_pct = commit_eff * 100.0
            commit_width_note = "assumed commit width=4" if assumed_width else f"commit width={commit_width}"
            commit_cycles_formula = (
                f"Commit cycles = {int(total_cycles):,} x ({ipc:.6f} / {commit_width}) ({commit_width_note})"
            )
            efficiency_formula = f"Efficiency = IPC / commit_width = {ipc:.6f} / {commit_width}"
            avg_commits_per_cycle = commit_eff * commit_width if commit_width > 0 else 0.0
            avg_commits_formula = "Avg commits/cycle = IPC (fallback)"

        metrics = {
          "total_cycles": f"{int(total_cycles):,}",
          "total_cycles_formula": f"Total cycles = simTicks = {p.get_raw('simTicks')}",
          "commit_cycles": f"{commit_cycles:,}",
          "commit_cycles_formula": commit_cycles_formula,
          "stall_cycles": f"{stall_cycles:,}",
          "stall_cycles_formula": f"Stall cycles = {int(total_cycles):,} - {commit_cycles:,}",
          "ipc": f"{ipc:.6f}",
          "ipc_formula": f"IPC = {p.get_raw('board.processor.cores.core.ipc', 'simInsts/simCycles')} (simInsts={p.get_raw('simInsts')})",
          "cpi": f"{cpi:.6f}",
          "cpi_formula": f"CPI = {p.get_raw('board.processor.cores.core.cpi', 'simCycles/simInsts')} (simTicks={p.get_raw('simTicks')})",
          "efficiency": f"{efficiency_pct:.2f}%",
          "efficiency_formula": efficiency_formula,
          "avg_commits_per_cycle": f"{avg_commits_per_cycle:.6f}",
          "avg_commits_per_cycle_formula": avg_commits_formula,
        }

        self._attach_pipeline_metrics(metrics)
        self._attach_bottleneck(metrics)
        self._attach_global_metrics(metrics, sim_seconds, sim_insts, sim_ops)

        return metrics

    def _attach_global_metrics(self, metrics: Dict[str, str], sim_seconds: float, sim_insts: float, sim_ops: float) -> None:
        metrics["sim_seconds"] = f"{sim_seconds:.6f}" if sim_seconds > 0 else "N/A"
        metrics["sim_insts"] = f"{int(sim_insts):,}" if sim_insts > 0 else "N/A"
        metrics["sim_ops"] = f"{int(sim_ops):,}" if sim_ops > 0 else "N/A"

    def _attach_pipeline_metrics(self, metrics: Dict[str, str]) -> None:
        p = self.parser
        fetch_run = int(p.get_value("board.processor.cores.core.fetch.status::running"))
        fetch_stall = int(p.get_value("board.processor.cores.core.fetch.status::icacheWaitResponse"))
        decode_run = int(p.get_value("board.processor.cores.core.decode.status::Running"))
        decode_block = int(p.get_value("board.processor.cores.core.decode.status::Blocked"))
        rename_stall = int(p.get_value("board.processor.cores.core.rename.status::SerializeStall"))
        commit_run = int(p.get_value("board.processor.cores.core.commit.status::running"))
        commit_trap = int(p.get_value("board.processor.cores.core.commit.status::trapPending"))

        metrics["fetch_running"] = f"{fetch_run:,}" if fetch_run else "N/A"
        metrics["fetch_stall"] = f"{fetch_stall:,}" if fetch_stall else "N/A"
        metrics["fetch_issue"] = f"ICache wait {fetch_stall:,} cycles" if fetch_stall else "N/A"

        metrics["decode_running"] = f"{decode_run:,}" if decode_run else "N/A"
        metrics["decode_stall"] = f"{decode_block:,}" if decode_block else "N/A"
        metrics["decode_issue"] = f"Blocked {decode_block:,} cycles" if decode_block else "N/A"

        metrics["rename_running"] = "N/A"
        metrics["rename_stall"] = f"{rename_stall:,}" if rename_stall else "N/A"
        metrics["rename_issue"] = f"Serialize stall {rename_stall:,} cycles" if rename_stall else "N/A"

        metrics["execute_running"] = "N/A"
        metrics["execute_stall"] = "N/A"
        metrics["execute_issue"] = "N/A"

        metrics["commit_running"] = f"{commit_run:,}" if commit_run else "N/A"
        metrics["commit_stall"] = f"{commit_trap:,}" if commit_trap else "N/A"
        metrics["commit_issue"] = f"Trap pending {commit_trap:,} cycles" if commit_trap else "N/A"

    def _attach_bottleneck(self, metrics: Dict[str, str]) -> None:
        p = self.parser
        total_cycles = p.get_value("board.processor.cores.core.numCycles") or p.get_value("simTicks")
        fetch_stall = int(p.get_value("board.processor.cores.core.fetch.status::icacheWaitResponse"))
        decode_block = int(p.get_value("board.processor.cores.core.decode.status::Blocked"))
        rename_stall = int(p.get_value("board.processor.cores.core.rename.status::SerializeStall"))

        l1d_miss_rate = p.get_value("board.cache_hierarchy.l1d-cache-0.demandMissRate::total")
        l2_miss_rate = p.get_value("board.cache_hierarchy.l2-cache-0.demandMissRate::total")

        bottleneck_type = "Back-end bound"
        cause = "Decode backpressure dominates available stall counters."
        support = f"decode.blocked={decode_block:,} cycles"

        if fetch_stall > decode_block and fetch_stall > rename_stall:
            bottleneck_type = "Front-end bound"
            cause = "Fetch is limited by ICache response latency."
            support = f"fetch.icacheWaitResponse={fetch_stall:,} cycles"
        elif rename_stall >= decode_block and rename_stall >= fetch_stall:
            bottleneck_type = "Rename / dependency bound"
            cause = "Rename serialize stalls dominate, indicating dependency pressure."
            support = f"rename.SerializeStall={rename_stall:,} cycles"
        else:
            if l2_miss_rate > 0.5 and l1d_miss_rate > 0:
                bottleneck_type = "Memory bound"
                cause = "High L2 miss rate suggests long-latency memory pressure."
                support = f"l2.missRate={l2_miss_rate:.6f}"

        metrics["bottleneck_type"] = bottleneck_type
        metrics["bottleneck_cause"] = cause
        metrics["bottleneck_support"] = support

    def _chart_data(self, metrics: Dict[str, str]) -> Dict[str, str]:
        commit_cycles_val = int(metrics["commit_cycles"].replace(",", "")) if metrics["commit_cycles"] != "N/A" else 0
        stall_cycles_val = int(metrics["stall_cycles"].replace(",", "")) if metrics["stall_cycles"] != "N/A" else 0

        stall_sources = self._stall_sources_top3()
        if stall_sources:
            labels = [s[0] for s in stall_sources]
            values = [s[1] for s in stall_sources]
            stall_chart_html = """
      <div class="chart-box">
        <canvas id="stallSourcesChart"></canvas>
      </div>
            """
            stall_chart_script = f"""
    const stallCtx = document.getElementById('stallSourcesChart').getContext('2d');
    new Chart(stallCtx, {{
      type: 'bar',
      data: {{
        labels: {json.dumps(labels)},
        datasets: [{{
          label: 'Cycles',
          data: {json.dumps(values)},
          backgroundColor: '#6b7280'
        }}]
      }},
      options: {{
        responsive: true,
        maintainAspectRatio: false,
        plugins: {{ legend: {{ display: false }} }},
        scales: {{ y: {{ beginAtZero: true }} }}
      }}
    }});
            """
        else:
            stall_chart_html = ""
            stall_chart_script = ""

        cache_chart_html, cache_chart_script = self._cache_chart()
        branch_chart_html, branch_chart_script = self._branch_chart()

        return {
          "commit_cycles_value": commit_cycles_val,
          "stall_cycles_value": stall_cycles_val,
          "stall_chart_html": stall_chart_html,
          "stall_chart_script": stall_chart_script,
          "cache_chart_html": cache_chart_html,
          "cache_chart_script": cache_chart_script,
          "branch_chart_html": branch_chart_html,
          "branch_chart_script": branch_chart_script,
        }

    def _stall_sources_top3(self) -> List[Tuple[str, int]]:
        p = self.parser
        candidates = [
            ("ICache wait", int(p.get_value("board.processor.cores.core.fetch.status::icacheWaitResponse"))),
            ("Decode blocked", int(p.get_value("board.processor.cores.core.decode.status::Blocked"))),
            ("Rename serialize stall", int(p.get_value("board.processor.cores.core.rename.status::SerializeStall"))),
            ("Commit trap pending", int(p.get_value("board.processor.cores.core.commit.status::trapPending"))),
        ]
        filtered = [(name, value) for name, value in candidates if value > 0]
        filtered.sort(key=lambda x: x[1], reverse=True)
        return filtered[:3]

    def _cache_chart(self) -> Tuple[str, str]:
        p = self.parser
        labels = ["L1D", "L1I", "L2"]
        hits = [
            int(p.get_value("board.cache_hierarchy.l1d-cache-0.demandHits::total")),
            int(p.get_value("board.cache_hierarchy.l1i-cache-0.demandHits::total")),
            int(p.get_value("board.cache_hierarchy.l2-cache-0.demandHits::total")),
        ]
        misses = [
            int(p.get_value("board.cache_hierarchy.l1d-cache-0.demandMisses::total")),
            int(p.get_value("board.cache_hierarchy.l1i-cache-0.demandMisses::total")),
            int(p.get_value("board.cache_hierarchy.l2-cache-0.demandMisses::total")),
        ]

        if sum(hits) + sum(misses) <= 0:
            return "", ""

        chart_html = """
      <div class="chart-box">
        <canvas id="cacheUsageChart"></canvas>
      </div>
        """
        chart_script = f"""
    const cacheCtx = document.getElementById('cacheUsageChart').getContext('2d');
    new Chart(cacheCtx, {{
      type: 'bar',
      data: {{
        labels: {json.dumps(labels)},
        datasets: [
          {{ label: 'Hits', data: {json.dumps(hits)}, backgroundColor: '#10b981' }},
          {{ label: 'Misses', data: {json.dumps(misses)}, backgroundColor: '#ef4444' }}
        ]
      }},
      options: {{
        responsive: true,
        maintainAspectRatio: false,
        plugins: {{ legend: {{ position: 'bottom' }} }},
        scales: {{ x: {{ stacked: true }}, y: {{ stacked: true, beginAtZero: true }} }}
      }}
    }});
        """
        return chart_html, chart_script

    def _branch_chart(self) -> Tuple[str, str]:
        summary = self._branch_summary()
        if not summary["available"]:
            return "", ""

        correct = summary["correct_count"]
        mispred = summary["mispred_count"]
        if correct + mispred <= 0:
            return "", ""

        chart_html = """
      <div class="chart-box">
        <canvas id="branchPredChart"></canvas>
      </div>
        """
        chart_script = f"""
    const branchCtx = document.getElementById('branchPredChart').getContext('2d');
    new Chart(branchCtx, {{
      type: 'pie',
      data: {{
        labels: ['Correct', 'Mispredicted'],
        datasets: [{{
          data: {json.dumps([correct, mispred])},
          backgroundColor: ['#3b82f6', '#f59e0b']
        }}]
      }},
      options: {{
        responsive: true,
        maintainAspectRatio: false,
        plugins: {{ legend: {{ position: 'bottom' }} }}
      }}
    }});
        """
        return chart_html, chart_script

    def _commit_histogram(self) -> Dict[str, float]:
        p = self.parser
        prefix = "board.processor.cores.core.commit.numCommittedDist::"
        total_samples = int(p.get_value(prefix + "samples"))
        total_field = int(p.get_value(prefix + "total"))

        counts: List[Tuple[int, int]] = []
        for key in p.keys():
          if not key.startswith(prefix):
            continue
          suffix = key[len(prefix):]
          if suffix.isdigit():
            count = int(p.get_value(key))
            counts.append((int(suffix), count))

        if total_samples <= 0:
          total_samples = total_field
        if total_samples <= 0 and counts:
          total_samples = sum(count for _, count in counts)

        weighted_sum = sum(n * count for n, count in counts)
        active_cycles = sum(count for n, count in counts if n > 0)
        avg_commits = (weighted_sum / total_samples) if total_samples > 0 else 0.0

        return {
          "total_cycles": float(total_samples),
          "active_cycles": float(active_cycles),
          "avg_commits": avg_commits,
        }

    def _branch_summary(self) -> Dict[str, str]:
        p = self.parser
        prefix = "board.processor.cores.core.branchPred."
        keys = [k for k in p.keys() if k.startswith(prefix)]
        if not keys:
          return {
            "available": False,
            "status": "N/A",
            "status_source": "No branch predictor stats found in stats.txt",
            "total_branches": "N/A",
            "total_branches_source": "N/A",
            "mispredictions": "N/A",
            "mispredictions_source": "N/A",
            "mispred_rate": "N/A",
            "mispred_rate_source": "N/A",
            "correct_count": 0,
            "mispred_count": 0,
          }

        total_key, total_val, total_raw = self._first_stat([
          prefix + "numBranches",
          prefix + "totalBranches",
          prefix + "lookups",
          prefix + "condPredicted",
        ])
        mispred_key, mispred_val, mispred_raw = self._first_stat([
          prefix + "mispredicted",
          prefix + "mispredictions",
          prefix + "condIncorrect",
        ])
        correct_key, correct_val, correct_raw = self._first_stat([
          prefix + "correct",
          prefix + "condCorrect",
        ])

        total = int(total_val)
        mispred = int(mispred_val)
        correct = int(correct_val)
        if total <= 0 and correct > 0 and mispred > 0:
          total = correct + mispred
        if correct <= 0 and total > 0 and mispred >= 0:
          correct = max(total - mispred, 0)

        mispred_rate = (mispred / total) if total > 0 else 0.0

        return {
          "available": True,
          "status": "Available",
          "status_source": f"{len(keys)} keys under {prefix}",
          "total_branches": f"{total:,}" if total > 0 else "N/A",
          "total_branches_source": total_key if total_key else "N/A",
          "mispredictions": f"{mispred:,}" if mispred > 0 else "0",
          "mispredictions_source": mispred_key if mispred_key else "N/A",
          "mispred_rate": f"{mispred_rate:.6f}" if total > 0 else "N/A",
          "mispred_rate_source": "mispredictions / total_branches",
          "correct_count": correct,
          "mispred_count": mispred,
        }

    def _first_stat(self, keys: List[str]) -> Tuple[str, float, str]:
        for key in keys:
          if key in self.parser.stats:
            entry = self.parser.stats[key]
            return key, entry.value, entry.raw_value
        return "", 0.0, "N/A"

    def _group_stats(self) -> Dict[str, List[Tuple[str, StatEntry]]]:
        stats = self.parser.stats
        all_keys = list(stats.keys())
        assigned = set()

        def collect(prefix: str) -> List[Tuple[str, StatEntry]]:
            items = [(k, stats[k]) for k in all_keys if k.startswith(prefix)]
            for k, _ in items:
                assigned.add(k)
            return items

        groups = {}
        global_keys = [
            "simSeconds", "simTicks", "finalTick", "simFreq",
            "hostSeconds", "hostTickRate", "hostMemory",
            "simInsts", "simOps", "hostInstRate", "hostOpRate",
        ]
        globals_list = []
        for key in global_keys:
            if key in stats:
                globals_list.append((key, stats[key]))
                assigned.add(key)
        if globals_list:
            groups["Global Metrics"] = globals_list

        branch = collect("board.processor.cores.core.branchPred.")
        if branch:
            groups["Branch Predictor Stats"] = branch

        core = collect("board.processor.cores.core.")
        if core:
            groups["CPU Core Stats"] = [item for item in core if item[0] not in {k for k, _ in branch}]

        cache = collect("board.cache_hierarchy.")
        if cache:
            groups["Cache Stats"] = cache

        mem = collect("board.memory.")
        if mem:
            groups["Memory Stats"] = mem

        other = [(k, stats[k]) for k in all_keys if k not in assigned]
        if other:
            groups["Other Stats"] = other

        return groups

    def _build_detailed_sections(self, groups: Dict[str, List[Tuple[str, StatEntry]]]) -> str:
        sections = []
        for name, items in groups.items():
            rows = []
            for key, entry in items:
                comment = entry.comment.replace("<", "&lt;").replace(">", "&gt;")
                rows.append(
                    f"<tr><td class=\"mono\">{key}</td><td class=\"mono\">{entry.raw_value}</td><td>{comment}</td></tr>"
                )
            table_html = (
                "<details open>"
                f"<summary>{name}</summary>"
                "<table>"
                "<tr><th>Key</th><th>Value</th><th>Description</th></tr>"
                + "".join(rows) +
                "</table></details>"
            )
            sections.append(table_html)
        return "".join(sections)

    def _cache_summary(self) -> Dict[str, str]:
        p = self.parser

        l1d_hits = int(p.get_value("board.cache_hierarchy.l1d-cache-0.demandHits::total"))
        l1d_misses = int(p.get_value("board.cache_hierarchy.l1d-cache-0.demandMisses::total"))
        l1d_accesses = int(p.get_value("board.cache_hierarchy.l1d-cache-0.demandAccesses::total"))
        l1d_miss_rate = p.get_value("board.cache_hierarchy.l1d-cache-0.demandMissRate::total")

        l1i_hits = int(p.get_value("board.cache_hierarchy.l1i-cache-0.demandHits::total"))
        l1i_misses = int(p.get_value("board.cache_hierarchy.l1i-cache-0.demandMisses::total"))
        l1i_accesses = int(p.get_value("board.cache_hierarchy.l1i-cache-0.demandAccesses::total"))
        l1i_miss_rate = p.get_value("board.cache_hierarchy.l1i-cache-0.demandMissRate::total")

        l2_hits = int(p.get_value("board.cache_hierarchy.l2-cache-0.demandHits::total"))
        l2_misses = int(p.get_value("board.cache_hierarchy.l2-cache-0.demandMisses::total"))
        l2_accesses = int(p.get_value("board.cache_hierarchy.l2-cache-0.demandAccesses::total"))
        l2_miss_rate = p.get_value("board.cache_hierarchy.l2-cache-0.demandMissRate::total")

        return {
          "l1d_hits": f"{l1d_hits:,}" if l1d_hits else "N/A",
          "l1d_misses": f"{l1d_misses:,}" if l1d_misses else "N/A",
          "l1d_accesses": f"{l1d_accesses:,}" if l1d_accesses else "N/A",
          "l1d_miss_rate": f"{l1d_miss_rate:.6f}" if l1d_miss_rate > 0 else "N/A",
          "l1i_hits": f"{l1i_hits:,}" if l1i_hits else "N/A",
          "l1i_misses": f"{l1i_misses:,}" if l1i_misses else "N/A",
          "l1i_accesses": f"{l1i_accesses:,}" if l1i_accesses else "N/A",
          "l1i_miss_rate": f"{l1i_miss_rate:.6f}" if l1i_miss_rate > 0 else "N/A",
          "l2_hits": f"{l2_hits:,}" if l2_hits else "N/A",
          "l2_misses": f"{l2_misses:,}" if l2_misses else "N/A",
          "l2_accesses": f"{l2_accesses:,}" if l2_accesses else "N/A",
          "l2_miss_rate": f"{l2_miss_rate:.6f}" if l2_miss_rate > 0 else "N/A",
        }


def main() -> None:
    import sys

    stats_file = sys.argv[1] if len(sys.argv) > 1 else "m5out/stats.txt"
    output_file = sys.argv[2] if len(sys.argv) > 2 else "gem5_o3_report.html"

    if not Path(stats_file).exists():
        print(f"Error: {stats_file} not found")
        sys.exit(1)

    parser = StatsParser(stats_file)
    report = ReportBuilder(parser).build()
    with open(output_file, "w") as f:
        f.write(report)

    print(f"✓ Performance report generated: {output_file}")


if __name__ == "__main__":
    main()
