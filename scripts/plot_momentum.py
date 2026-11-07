import csv
import sys

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
"""edited by kenneth"""

"""ploting momentum by the minute by the monmentum tracker"""
def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    src, out = sys.argv[1], sys.argv[2]
    window = sys.argv[3] if len(sys.argv) > 3 else "5"
    with open(src, encoding = "utf-8") as f:
        title = f.readline().lstrip("# ").strip()
        rows = list(csv.DictReader(f))
    if not rows:
        sys.exit("No rows found in the input file")
    home, away = rows[0]["home_team"], rows[0]["away_team"]
    minute = [int(row["minute"]) for row in rows]
    net = [ float(row["net_score"]) for row in rows]
    cum_home = [float(row["cum_home"]) for row in rows]
    cum_away = [float(row["cum_away"]) for row in rows]

    fig, (ax, ax2) = plt.subplots(2, 1, figsize = (13, 8), sharex = True, gridspec_kw = {"height_ratios": [3, 2]})
    ax.plot(minute, net, color = "black", where = [v == 0 for v in net], label = f"Net momentum({window} minute rolling window)")
    ax.fill_between(minute, net, 0, where=[v < 0 for v in net], color = "tab:red", alpha = 0.35, interpolate= True, label = f"{away} momentum")
    ax.axhline(0, color="gray", linewidth=0.8)

    first = True
    for r, m, v, in zip(rows, minute, net):
        s = int(r["minute"])
        if s:
            ax.scatter([m], [v], marker="^" if s > 0 else "v", s=110, color="tab:blue" if s > 0 else "tab:red",edgecolor="black", zorder = 5, label = "Momentum shift" if first else None)
            ax.annotate(f"{m}'", (m, v), textcoords="offset points", xytext=(0, 10 if s > 0 else -16), ha="center", fontsize=8)
            first = False
        for r, m in zip(rows, minute):
            for k, col, who in (("goals_home", "tab:blue", home), ("goals_away", "tab:red", away)):
                if int(r[k]):
                    for a in (ax, ax2):
                        a.axvline(m - 0.5, color=col, linestyle="--", linewidth=1)
                    ax.text(m - 0.5, ax.get_ylim()[1] * 0.92, f"GOAL {who}", rotation=90, va = "top", ha = "right",fontsize = 8, color = col)
    ax.set_ylabel("Momentum score\n(+ {home} / - {away})")
    ax.set_title(f"Momentum{title}")
    ax.legend(loc = "lower left", fontsize = 8, ncol = 2)
    ax.grid(alpha = 0.3)

    ax2.plot(minute, cum_home, color = "tab:blue", label = f"{home} momentum")
    ax2.plot(minute, cum_away, color = "tab:red", label = f"{away} momentum")
    ax2.set_ylabel("Cumulative momentum")
    ax2.set_xlabel("Minute match")
    ax2.set_xlim(ax.get_xlim())
    ax2.legend(loc = "upper left", fontsize = 8)
    ax2.grid(alpha = 0.3)
    for t in (45, 90, 105):
        if minute[-1] > t:
            for a in (ax, ax2):
                a.axvline(t, color = "gray", linestyle = ":", linewidth = 1)

    fig.tight_layout()
    fig.savefig(out, dpi = 130)
    print(f"plot written: {out}")

