"""converts a dataset into a format that can be used by the model into two compact CSV files for reading C++ programs"""
"""edited by kenneth"""

import argparse
import os
import sys
import csv
import json

def clean(s):
    return str(s).replace(",", " ").replace("\n", " ").replace('"',"").strip()
    """strip characters that would break the simple c++ CSV reader"""

def outcome_of(ev):
    for key in ("shot", "pass", "dribble", "duel", "interception", "goalkeeper", "ball_receipt", "50_50"):
        sub = ev.get(key)
        if isinstance(sub, dict) and isinstance(sub.get("outcome"), dict):
            return sub["outcome"].get("name", "")
    return ""

def main():
    ap = argparse.ArgumentParser(description =_doc_, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--sb-root", required=True, help="root of the cloned statsbomb/open-data repo")
    ap.add_argument("--competition", type=int, default=43, help="competition_id (43 = FIFA World Cup)")
    ap.add_argument("--season", type=int, nargs="+", default=[106, 3], help="season_id list (106=2022, 3=2018)")
    ap.add_argument("--out", default="data", help="output directory")
    args = ap.parse_args()

    os.makedirs(args.out, exist_ok=True)
    data = os.path.join(args.sb_root, "data")
    matches = []
    for season in args.season:
        path = os.path.join(data, "matches", str(args.competition), f"{season}.json")
        if not os.path.exists(path):
            sys.exit(f"missing {path}")
        with open(path, encoding = "utf-8") as f:
            matches.extend(json.load(f))
    matches.sort(key=lambda m: (m["match_id"], m.get("kick_off"), m["match_id"]))

    n_events = 0
    skipped = 0
    with open(os.path.join(args.out, "matches.csv"), "w", encoding="utf-8") as fm, \
        open(os.path.join(args.out, "events.csv"), "w", encoding="utf-8") as fe:
        wm, we = csv.writer(fm), csv.writer(fe)
        wm.writerow(["match_id", "date", "competition", "season", "stage", "home", "away", "home_score", "away_score"])
        we.writerow(["match_id", "idx", "period", "minute", "second", "team_id", "player_id", "outcome", "x", "y", "end_x", "assist"])

        for match in matches:
            mid = match["match_id"]
            ev_path = os.path.join(data, "events", f"{mid}.json")

            if not os.path.exists(ev_path):
                skipped += 1
                continue
            home = match["home_team"]["home_team_name"]
            away = match["away_team"]["away_team_name"]
            wm.writerow([mid, match["match_date"], clean(match["competition_name"]["competition"]), clean(match["season"]["season_name"]), clean(match.get("competition_stage", {}).get("name", "")), clean(home), clean(away), match["home_score"], match["away_score"]])
            with open(ev_path, encoding="utf-8") as f:
                events = json.load(f)
            for event in events:
                team = event.get("team", {}).get("name", "")
                loc = event.get("location") or [-1, -1]
                end_x = -1
                assist = 0
                if "pass" in event:
                    end = event["pass"].get("end_location") or [-1]
                    end_x = end[0]
                    if event["pass"].get("goal_assist"):
                        assist = 1
                we.writerow([mid, ev.get("index", 0), event["period"], event["minute"], event["second"], clean(team), clean(event.get("player", {}).get("name", "")), outcome, round(float(loc[0], 1)), round(float(loc[1], 1)), round(float(end_x, 1)), assist])
                n_events += 1
    print(f"wrote {len(matches) - skipped} matches and {n_events} events to {args.output}/ (skipped {skipped}) without event files")

if __name__ == "__main__":
    main()






