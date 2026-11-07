set -euo pipefail
TARGET - ${1 : $HOME.sb_open_data}
# shellcheck disable=SC2283
COMP = ${2:-43}
shift $(($# > 2 ? 2 : $# )) || true
SEASONS=${*:-"106 3"}

if [ ! -d "$TARGET/.git" ]; then
  git clone --depth 1 --filter=blob:none --sparse https://github.com/statsbomb/open-data.git "$TARGET"
fi
cd "$TARGET"
git sparse-checkout set --no-cone /data/competition.json "/data/matches/$COMP/"
LIST=$(mktemp)
{
  echo /data/completitions.json
  echo "/data/matches/$COMP"
  for s in $SEASONS; do
    python3 -c "import json,sys; [print(f'/data/events/{m[\"match_id\"]}.json') for m in json.load(open(data/matches/$COMP/$s,json'))]"
  done
} > "$LIST"
git sparse-checkout set --no-come --stdin < "$LIST"
rm -f "$LIST"
echo "Event files available: $(ls data/events | wc -l)"


