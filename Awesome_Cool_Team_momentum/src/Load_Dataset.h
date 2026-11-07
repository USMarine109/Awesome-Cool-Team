// made by aidan
#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Fenwick_Tree.h"
#include "Segment_Tree.h"

struct Event {
    int matchId = 0;
    int index = 0;
    int period = 0;
    int minute = 0;
    int second = 0;

    bool isHome = false;

    std::string team;
    std::string type;
    std::string outcome;

    float x = -1;
    float y = -1;
    float endX = -1;

    int assist = 0;

    int clockSeconds() const {
        return minute * 60 + second;
    }
};

namespace momentum {

constexpr long long SCALE = 2;

long long halfPoints(const Event& e);
const char* reason(const Event& e);

inline double toPoints(long long half) {
    return static_cast<double>(half) / SCALE;
}

}

struct PeriodRange {
    int period;
    std::size_t begin;
    std::size_t end;
};

struct Match {
    int id = 0;

    std::string date;
    std::string competition;
    std::string season;
    std::string stage;
    std::string home;
    std::string away;

    int homeScore = 0;
    int awayScore = 0;

    std::size_t begin = 0;
    std::size_t end = 0;

    std::vector<PeriodRange> periods;

    int lastMinute = 0;

    std::string label() const;
};

enum class Structure {
    Segment,
    Fenwick,
    Naive
};

struct RangeResult {
    long long home = 0;
    long long away = 0;

    std::size_t events = 0;

    long long net() const {
        return home - away;
    }
};

struct MinuteSample {
    int minute;

    long long home;
    long long away;

    long long cumHome;
    long long cumAway;

    int shift;
    int goalsHome;
    int goalsAway;
};

struct Shift {
    int minute;
    int toward;
    long long net;
};

struct ShiftReport {
    std::vector<MinuteSample> series;
    std::vector<Shift> shifts;
    std::vector<MinuteSample> peaks;
};

struct BenchRow {
    std::size_t n;

    double segBuildMs;
    double fenBuildMs;

    double segQueryNs;
    double fenQueryNs;
    double naiveQueryNs;

    double segUpdateNs;
    double fenUpdateNs;

    std::size_t segBytes;
    std::size_t fenBytes;
    std::size_t rawBytes;

    std::size_t verified;
    bool allAgree;
};

class Tracker {
public:
    bool load(
        const std::string& eventsCsv,
        const std::string& matchesCsv,
        std::string& err
    );

    bool loaded() const {
        return !events_.empty();
    }

    std::size_t eventCount() const {
        return events_.size();
    }

    const std::vector<Match>& matches() const {
        return matches_;
    }

    const std::vector<Event>& events() const {
        return events_;
    }

    const std::vector<long long>& homeValues() const {
        return home_;
    }

    const std::vector<long long>& awayValues() const {
        return away_;
    }

    int findMatch(int matchIdOrIndex) const;

    double buildSegment();
    double buildFenwick();

    bool segmentBuilt() const {
        return segBuilt_;
    }

    bool fenwickBuilt() const {
        return fenBuilt_;
    }

    RangeResult queryMinutes(
        int m,
        int fromMin,
        int toMin,
        Structure s
    ) const;

    void updateEvent(
        std::size_t i,
        long long newHalfPoints
    );

    ShiftReport detectShifts(
        int m,
        int window,
        double thresholdPoints,
        Structure s
    ) const;

    std::vector<BenchRow> benchmark(
        std::size_t queries,
        std::size_t updates,
        unsigned seed
    ) const;

    bool exportSeriesCsv(
        int m,
        const ShiftReport& r,
        const std::string& path
    ) const;

private:
    void periodSlice(
        const PeriodRange& p,
        int fromSec,
        int toSec,
        std::size_t& lo,
        std::size_t& hi
    ) const;

    std::vector<Event> events_;
    std::vector<int> clock_;

    std::vector<long long> home_;
    std::vector<long long> away_;

    std::vector<Match> matches_;

    SegmentTree<long long> segHome_;
    SegmentTree<long long> segAway_;

    FenwickTree<long long> fenHome_;
    FenwickTree<long long> fenAway_;

    bool segBuilt_ = false;
    bool fenBuilt_ = false;
};
