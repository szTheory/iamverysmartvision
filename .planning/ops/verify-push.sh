#!/bin/sh
# usage: verify.sh <base-sha>  (orchestrator: trailers, gate, push, hosted run, record row)
cd "$(git rev-parse --show-toplevel)" || exit 1
base=$1
BR=$(git branch --show-current)
n=$(git log --oneline "$base"..HEAD | wc -l | tr -d ' ')
t=$(git log --format=%b "$base"..HEAD | grep -c 'Co-Authored-By: Claude Sonnet 5.5 <noreply@anthropic.com>')
echo "commits=$n trailers=$t"
[ -z "$LOCAL_GATE" ] || sh tools/gate.sh 2>&1 | grep -a 'FAIL\|gate: ' | cut -c1-70
git push -q origin $BR 2>&1 | tail -1
head=$(git rev-parse HEAD)
id=
i=0
while [ -z "$id" ] && [ $i -lt 30 ]; do
  id=$(gh run list --branch $BR --limit 5 --json databaseId,headSha -q ".[] | select(.headSha==\"$head\") | .databaseId" | head -1)
  [ -n "$id" ] || { sleep 10; i=$((i+1)); }
done
[ -n "$id" ] || { echo "no run found for HEAD"; exit 1; }
gh run watch "$id" --exit-status > /dev/null 2>&1; echo "run=$id rc=$?"
row=$(sh tools/ci-record.sh "$id" 2>/dev/null | tail -1)
[ -n "$row" ] || { echo "no row"; exit 1; }
echo "$row" >> .planning/ci-runs.tsv
(head -1 .planning/ci-runs.tsv; tail -n +2 .planning/ci-runs.tsv | sort -u | sort -k1,1 -s) > "$0.tmp" && cp "$0.tmp" .planning/ci-runs.tsv
python3 - <<'PY'
import statistics as st
rows=[l.split('\t') for l in open('.planning/ci-runs.tsv').read().strip().split('\n')[1:]]
ok=[r for r in rows if r[4]=='success'][-5:]
print('last row:',rows[-1][4],'gate',rows[-1][9],'cpu',rows[-1][12],'| last5 medians gate',st.median(int(r[9]) for r in ok),'cpu',st.median(int(r[12]) for r in ok))
PY
git add .planning/ci-runs.tsv && git commit -q -m "docs(ci): record hosted run $id

Co-Authored-By: Claude Sonnet 5.5 <noreply@anthropic.com>"
