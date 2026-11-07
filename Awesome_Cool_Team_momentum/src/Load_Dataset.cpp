
#include "Load_Dataset.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace momentum {
namespace {
    constexpr float FINAL_THIRD_X = 80.0f;

    bool isGoal(const Event& e) {
        return (e.type == "Shot" && e.outcome == "Goal") ||
               e.type == "Own Goal For";
    }

    bool isOnTargetShot(const Event& e) {
        return e.type == "Shot" &&
               (e.outcome == "Saved" ||
                e.outcome == "Saved to Post");
    }

    bool isDangerousPass(const Event& e) {
        if (e.type != "Pass" || !e.outcome.empty())
            return false;

        if (e.assist > 0)
            return true;

        return e.x >= 0 && e.x < FINAL_THIRD_X &&
               e.endX >= FINAL_THIRD_X;
    }

    bool isTurnover(const Event& e) {
        return e.type == "Dispossessed" ||
               e.type == "Miscontrol" ||
               (e.type == "Dribble" &&
                e.outcome == "Incomplete");
    }
}

long long halfPoints(const Event& e) {
    if (e.period >= 5) return 0;
    if (isGoal(e)) return 6;
    if (isOnTargetShot(e)) return 2;
    if (isDangerousPass(e)) return 1;
    if (isTurnover(e)) return -2;
    return 0;
}

const char* reason(const Event& e) {
    if (e.period >= 5) return "";
    if (isGoal(e)) return "goal";
    if (isOnTargetShot(e)) return "shot on target";
    if (isDangerousPass(e)) return "dangerous pass";
    if (isTurnover(e)) return "turnover";
    return "";
}
}

namespace {
    using Clock = std::chrono::steady_clock;

    double msSince(Clock::time_point start) {
        return std::chrono::duration<double, std::milli>(
            Clock::now() - start).count();
    }

    std::vector<std::string> splitCsv(const std::string& line) {
        std::vector<std::string> fields;
        std::string current;

        for (char c : line) {
            if (c == ',') {
                fields.push_back(current);
                current.clear();
            } else if (c != '\r') {
                current += c;
            }
        }

        fields.push_back(current);
        return fields;
    }

    long long naiveSum(const std::vector<long long>& a,
                       std::size_t l, std::size_t r) {
        long long sum = 0;
        for (std::size_t i = l; i <= r; ++i)
            sum += a[i];
        return sum;
    }
}

std::string Match::label() const {
    std::ostringstream os;
    os << home << " " << homeScore << "-"
       << awayScore << " " << away
       << " (" << competition << " " << season
       << ", " << stage << ", " << date << ")";
    return os.str();
}

bool Tracker::load(const std::string& eventsCsv,
                   const std::string& matchesCsv,
                   std::string& err) {
    std::ifstream fm(matchesCsv);
    std::ifstream fe(eventsCsv);

    if (!fm) {
        err = "Cannot open " + matchesCsv;
        return false;
    }
    if (!fe) {
        err = "Cannot open " + eventsCsv;
        return false;
    }

    std::vector<Match> matches;
    std::unordered_map<int, std::size_t> matchPos;
    std::string line;

    std::getline(fm, line);

    while (std::getline(fm, line)) {
        auto f = splitCsv(line);
        if (f.size() < 9) continue;

        Match m;
        m.id = std::stoi(f[0]);
        m.date = f[1];
        m.competition = f[2];
        m.season = f[3];
        m.stage = f[4];
        m.home = f[5];
        m.away = f[6];
        m.homeScore = std::stoi(f[7]);
        m.awayScore = std::stoi(f[8]);

        matchPos[m.id] = matches.size();
        matches.push_back(std::move(m));
    }

    std::vector<std::vector<Event>> buckets(matches.size());

    std::getline(fe, line);

    while (std::getline(fe, line)) {
        auto f = splitCsv(line);
        if (f.size() < 13) {
            err = "Malformed events CSV";
            return false;
        }

        Event e;
        e.matchId = std::stoi(f[0]);

        auto it = matchPos.find(e.matchId);
        if (it == matchPos.end()) continue;

        e.index = std::stoi(f[1]);
        e.period = std::stoi(f[2]);
        e.minute = std::stoi(f[3]);
        e.second = std::stoi(f[4]);
        e.team = f[5];
        e.isHome = f[6] == "1";
        e.type = f[7];
        e.outcome = f[8];
        e.x = std::stof(f[9]);
        e.y = std::stof(f[10]);
        e.endX = std::stof(f[11]);
        e.assist = std::stoi(f[12]);

        buckets[it->second].push_back(std::move(e));
    }

    events_.clear();
    clock_.clear();
    home_.clear();
    away_.clear();
    matches_.clear();
    segBuilt_ = fenBuilt_ = false;

    for (std::size_t k = 0; k < matches.size(); ++k) {
        auto& bucket = buckets[k];
        if (bucket.empty()) continue;

        std::stable_sort(bucket.begin(), bucket.end(),
            [](const Event& a, const Event& b) {
                if (a.period != b.period)
                    return a.period < b.period;
                if (a.clockSeconds() != b.clockSeconds())
                    return a.clockSeconds() < b.clockSeconds();
                return a.index < b.index;
            });

        Match m = matches[k];
        m.begin = events_.size();

        for (auto& e : bucket) {
            std::size_t i = events_.size();

            if (m.periods.empty() ||
                m.periods.back().period != e.period) {
                m.periods.push_back({e.period, i, i});
            }

            m.periods.back().end = i + 1;

            if (e.period < 5)
                m.lastMinute = std::max(
                    m.lastMinute, e.minute + 1);

            long long points = momentum::halfPoints(e);

            home_.push_back(e.isHome ? points : 0);
            away_.push_back(e.isHome ? 0 : points);
            clock_.push_back(e.clockSeconds());
            events_.push_back(std::move(e));
        }

        m.end = events_.size();
        matches_.push_back(std::move(m));
    }

    if (events_.empty()) {
        err = "No events loaded";
        return false;
    }

    return true;
}

int Tracker::findMatch(int key) const {
    for (std::size_t i = 0; i < matches_.size(); ++i) {
        if (matches_[i].id == key)
            return static_cast<int>(i);
    }

    if (key >= 1 &&
        static_cast<std::size_t>(key) <= matches_.size())
        return key - 1;

    return -1;
}

double Tracker::buildSegment() {
    auto start = Clock::now();
    segHome_.build(home_);
    segAway_.build(away_);
    segBuilt_ = true;
    return msSince(start);
}

double Tracker::buildFenwick() {
    auto start = Clock::now();
    fenHome_.build(home_);
    fenAway_.build(away_);
    fenBuilt_ = true;
    return msSince(start);
}

void Tracker::periodSlice(const PeriodRange& p,
                          int fromSec, int toSec,
                          std::size_t& lo,
                          std::size_t& hi) const {
    auto first = clock_.begin() + p.begin;
    auto last = clock_.begin() + p.end;

    lo = std::lower_bound(first, last, fromSec)
         - clock_.begin();

    hi = std::lower_bound(first, last, toSec)
         - clock_.begin();
}

RangeResult Tracker::queryMinutes(int m, int fromMin,
                                  int toMin, Structure s) const {
    RangeResult result;

    if (m < 0 ||
        static_cast<std::size_t>(m) >= matches_.size() ||
        fromMin >= toMin)
        return result;

    for (const auto& p : matches_[m].periods) {
        std::size_t lo, hi;

        periodSlice(p, fromMin * 60, toMin * 60,
                    lo, hi);

        if (lo >= hi) continue;

        result.events += hi - lo;

        switch (s) {
            case Structure::Segment:
                result.home += segHome_.query(lo, hi - 1);
                result.away += segAway_.query(lo, hi - 1);
                break;

            case Structure::Fenwick:
                result.home += fenHome_.query(lo, hi - 1);
                result.away += fenAway_.query(lo, hi - 1);
                break;

            case Structure::Naive:
                result.home += naiveSum(home_, lo, hi - 1);
                result.away += naiveSum(away_, lo, hi - 1);
                break;
        }
    }

    return result;
}

void Tracker::updateEvent(std::size_t i,
                          long long newHalfPoints) {
    if (i >= events_.size())
        throw std::out_of_range("Invalid event index");

    long long h = events_[i].isHome ? newHalfPoints : 0;
    long long a = events_[i].isHome ? 0 : newHalfPoints;

    if (segBuilt_) {
        segHome_.set(i, h);
        segAway_.set(i, a);
    }

    if (fenBuilt_) {
        fenHome_.set(i, h);
        fenAway_.set(i, a);
    }

    home_[i] = h;
    away_[i] = a;
}

ShiftReport Tracker::detectShifts(
    int m, int window, double thresholdPoints,
    Structure s) const {

    ShiftReport report;

    if (m < 0 ||
        static_cast<std::size_t>(m) >= matches_.size() ||
        window <= 0)
        return report;

    const Match& match = matches_[m];

    long long threshold = std::llround(
        thresholdPoints * momentum::SCALE);

    std::vector<int> gh(match.lastMinute + 1, 0);
    std::vector<int> ga(match.lastMinute + 1, 0);

    for (std::size_t i = match.begin;
         i < match.end; ++i) {
        const Event& e = events_[i];

        if (std::string(momentum::reason(e)) == "goal" &&
            e.minute <= match.lastMinute) {
            if (e.isHome) gh[e.minute]++;
            else ga[e.minute]++;
        }
    }

    int dominant = 0;
    long long cumH = 0, cumA = 0;

    for (int minute = 1;
         minute <= match.lastMinute; ++minute) {

        RangeResult w = queryMinutes(
            m, std::max(0, minute - window), minute, s);

        RangeResult one = queryMinutes(
            m, minute - 1, minute, s);

        cumH += one.home;
        cumA += one.away;

        MinuteSample sample{
            minute, w.home, w.away,
            cumH, cumA, 0,
            gh[minute - 1], ga[minute - 1]
        };

        long long net = w.net();

        int sign = net >= threshold ? 1 :
                   net <= -threshold ? -1 : 0;

        if (sign != 0 && sign != dominant) {
            if (dominant != 0) {
                sample.shift = sign;
                report.shifts.push_back({
                    minute, sign, net
                });
            }
            dominant = sign;
        }

        report.series.push_back(sample);
    }

    auto byHome = report.series;
    auto byAway = report.series;

    auto netOf = [](const MinuteSample& x) {
        return x.home - x.away;
    };

    std::sort(byHome.begin(), byHome.end(),
        [&](const auto& a, const auto& b) {
            return netOf(a) > netOf(b);
        });

    std::sort(byAway.begin(), byAway.end(),
        [&](const auto& a, const auto& b) {
            return netOf(a) < netOf(b);
        });

    auto takePeaks = [&](const auto& values, int dir) {
        std::vector<int> used;

        for (const auto& x : values) {
            if (used.size() == 3 ||
                netOf(x) * dir <= 0)
                break;

            bool near = false;
            for (int u : used)
                if (std::abs(u - x.minute) < window)
                    near = true;

            if (near) continue;

            used.push_back(x.minute);
            report.peaks.push_back(x);
        }
    };

    takePeaks(byHome, 1);
    takePeaks(byAway, -1);

    return report;
}

bool Tracker::exportSeriesCsv(
    int m, const ShiftReport& report,
    const std::string& path) const {

    if (m < 0 ||
        static_cast<std::size_t>(m) >= matches_.size())
        return false;

    std::ofstream out(path);
    if (!out) return false;

    const Match& match = matches_[m];

    out << "# " << match.label() << "\n";
    out << "minute,home,away,net,cum_home,cum_away,"
           "shift,goals_home,goals_away,home_team,away_team\n";

    for (const auto& x : report.series) {
        out << x.minute << ","
            << momentum::toPoints(x.home) << ","
            << momentum::toPoints(x.away) << ","
            << momentum::toPoints(x.home - x.away) << ","
            << momentum::toPoints(x.cumHome) << ","
            << momentum::toPoints(x.cumAway) << ","
            << x.shift << ","
            << x.goalsHome << ","
            << x.goalsAway << ","
            << match.home << ","
            << match.away << "\n";
    }

    return static_cast<bool>(out);
}

std::vector<BenchRow> Tracker::benchmark(
    std::size_t queries,
    std::size_t updates,
    unsigned seed) const {

    std::vector<BenchRow> rows;
    std::size_t N = events_.size();

    if (N == 0) return rows;

    std::vector<std::size_t> sizes;

    for (std::size_t s : {std::size_t(10000),
                          std::size_t(100000)}) {
        if (s < N) sizes.push_back(s);
    }

    sizes.push_back(N);

    for (std::size_t n : sizes) {
        std::vector<long long> a(n);

        for (std::size_t i = 0; i < n; ++i)
            a[i] = home_[i] - away_[i];

        BenchRow row{};
        row.n = n;

        SegmentTree<long long> seg;
        FenwickTree<long long> fen;

        auto start = Clock::now();
        for (int i = 0; i < 5; ++i)
            seg.build(a);
        row.segBuildMs = msSince(start) / 5;

        start = Clock::now();
        for (int i = 0; i < 5; ++i)
            fen.build(a);
        row.fenBuildMs = msSince(start) / 5;

        std::mt19937 rng(seed);
        std::uniform_int_distribution<std::size_t>
            pick(0, n - 1);

        std::vector<std::pair<std::size_t, std::size_t>>
            ranges(queries);

        for (auto& q : ranges) {
            q = {pick(rng), pick(rng)};
            if (q.first > q.second)
                std::swap(q.first, q.second);
        }

        volatile long long sink = 0;
        long long total = 0;

        start = Clock::now();
        for (const auto& q : ranges)
            total += seg.query(q.first, q.second);
        row.segQueryNs = queries ?
            msSince(start) * 1e6 / queries : 0;
        sink = total;

        total = 0;
        start = Clock::now();
        for (const auto& q : ranges)
            total += fen.query(q.first, q.second);
        row.fenQueryNs = queries ?
            msSince(start) * 1e6 / queries : 0;
        sink = total;

        std::size_t checks =
            std::min<std::size_t>(queries, 2000);

        total = 0;
        start = Clock::now();

        for (std::size_t i = 0; i < checks; ++i)
            total += naiveSum(
                a, ranges[i].first, ranges[i].second);

        row.naiveQueryNs = checks ?
            msSince(start) * 1e6 / checks : 0;
        sink = total;

        bool agree = true;

        for (std::size_t i = 0; i < checks; ++i) {
            auto [l, r] = ranges[i];
            long long expected = naiveSum(a, l, r);

            if (seg.query(l, r) != expected ||
                fen.query(l, r) != expected)
                agree = false;
        }

        std::uniform_int_distribution<int> delta(-6, 6);

        std::vector<std::pair<std::size_t, long long>>
            changes(updates);

        for (auto& u : changes)
            u = {pick(rng), delta(rng)};

        start = Clock::now();
        for (const auto& u : changes)
            seg.add(u.first, u.second);

        row.segUpdateNs = updates ?
            msSince(start) * 1e6 / updates : 0;

        start = Clock::now();
        for (const auto& u : changes)
            fen.add(u.first, u.second);

        row.fenUpdateNs = updates ?
            msSince(start) * 1e6 / updates : 0;

        for (const auto& u : changes)
            a[u.first] += u.second;

        for (std::size_t i = 0; i < checks; ++i) {
            std::size_t l = pick(rng);
            std::size_t r = pick(rng);

            if (l > r) std::swap(l, r);

            long long expected = naiveSum(a, l, r);

            if (seg.query(l, r) != expected ||
                fen.query(l, r) != expected)
                agree = false;
        }

        (void)sink;

        row.verified = 2 * checks;
        row.allAgree = agree;
        row.segBytes = seg.memoryBytes();
        row.fenBytes = fen.memoryBytes();
        row.rawBytes = a.capacity() * sizeof(long long);

        rows.push_back(row);
    }

    return rows;
}
