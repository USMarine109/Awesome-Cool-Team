
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "Load_Dataset.h"

using namespace std;

struct Options {
    string eventsCsv = "data/events.csv";
    string matchesCsv = "data/matches.csv";
    string plotScript = "scripts/plot_momentum.py";
    string outDir = "output";

    int window = 5;
    double threshold = 2.0;

    size_t queries = 1000000;
    size_t updates = 1000000;
    unsigned seed = 42;
};

struct Session {
    Options opt;
    Tracker tracker;
    int match = -1;
};

string fmt(double value, int precision = 2) {
    ostringstream os;
    os << fixed << setprecision(precision) << value;
    return os.str();
}

string pts(long long half) {
    return fmt(momentum::toPoints(half), 1);
}

bool loadDataset(Session& s) {
    string error;

    cout << "Loading dataset...\n";

    auto start = chrono::steady_clock::now();

    if (!s.tracker.load(
            s.opt.eventsCsv,
            s.opt.matchesCsv,
            error)) {
        cout << "Error: " << error << "\n";
        return false;
    }

    double ms = chrono::duration<double, milli>(
        chrono::steady_clock::now() - start).count();

    cout << "Loaded " << s.tracker.eventCount()
         << " events from "
         << s.tracker.matches().size()
         << " matches in " << fmt(ms) << " ms.\n";

    s.match = -1;
    return true;
}

void listMatches(const Session& s) {
    const auto& matches = s.tracker.matches();

    for (size_t i = 0; i < matches.size(); ++i) {
        cout << i + 1 << ". ["
             << matches[i].id << "] "
             << matches[i].label() << "\n";
    }
}

bool selectMatch(Session& s, int key) {
    int index = s.tracker.findMatch(key);

    if (index < 0) {
        cout << "Match not found.\n";
        return false;
    }

    s.match = index;

    cout << "Selected: "
         << s.tracker.matches()[index].label()
         << "\n";

    return true;
}

void buildSegment(Session& s) {
    double ms = s.tracker.buildSegment();

    cout << "Segment Tree built in "
         << fmt(ms, 3) << " ms.\n";
}

void buildFenwick(Session& s) {
    double ms = s.tracker.buildFenwick();

    cout << "Fenwick Tree built in "
         << fmt(ms, 3) << " ms.\n";
}

Structure preferredTree(const Session& s) {
    return s.tracker.segmentBuilt()
        ? Structure::Segment
        : Structure::Fenwick;
}

bool ready(const Session& s) {
    if (!s.tracker.loaded()) {
        cout << "Load the dataset first.\n";
        return false;
    }

    if (s.match < 0) {
        cout << "Select a match first.\n";
        return false;
    }

    if (!s.tracker.segmentBuilt() &&
        !s.tracker.fenwickBuilt()) {
        cout << "Build a tree first.\n";
        return false;
    }

    return true;
}

void queryMomentum(Session& s, int from, int to) {
    if (!ready(s)) return;

    const Match& match = s.tracker.matches()[s.match];

    RangeResult naive = s.tracker.queryMinutes(
        s.match, from, to, Structure::Naive);

    cout << "\nMomentum from minute "
         << from << " to " << to << "\n";

    auto display = [&](const string& name, Structure type) {
        auto start = chrono::steady_clock::now();

        RangeResult r = s.tracker.queryMinutes(
            s.match, from, to, type);

        double us = chrono::duration<double, micro>(
            chrono::steady_clock::now() - start).count();

        cout << name << ":\n";
        cout << "  " << match.home << ": "
             << pts(r.home) << "\n";
        cout << "  " << match.away << ": "
             << pts(r.away) << "\n";
        cout << "  Net momentum: "
             << pts(r.net()) << "\n";
        cout << "  Events: " << r.events << "\n";
        cout << "  Query time: " << fmt(us)
             << " microseconds\n";

        cout << "  Verification: "
             << ((r.home == naive.home &&
                  r.away == naive.away)
                     ? "PASS" : "FAIL")
             << "\n";
    };

    if (s.tracker.segmentBuilt())
        display("Segment Tree", Structure::Segment);

    if (s.tracker.fenwickBuilt())
        display("Fenwick Tree", Structure::Fenwick);

    if (naive.net() > 0)
        cout << match.home << " has momentum.\n";
    else if (naive.net() < 0)
        cout << match.away << " has momentum.\n";
    else
        cout << "Momentum is tied.\n";
}

ShiftReport detectMomentumShifts(Session& s) {
    ShiftReport report;

    if (!ready(s)) return report;

    const Match& match = s.tracker.matches()[s.match];

    report = s.tracker.detectShifts(
        s.match,
        s.opt.window,
        s.opt.threshold,
        preferredTree(s));

    cout << "\nMomentum Shifts\n";
    cout << "Window: " << s.opt.window << " minutes\n";
    cout << "Threshold: " << s.opt.threshold << "\n";

    for (const auto& shift : report.shifts) {
        cout << "Minute " << shift.minute
             << ": Momentum shifts to "
             << (shift.toward > 0 ? match.home : match.away)
             << " (net " << pts(shift.net) << ")\n";
    }

    if (report.shifts.empty())
        cout << "No momentum shifts detected.\n";

    cout << "\nStrongest Momentum Periods\n";

    for (const auto& peak : report.peaks) {
        long long net = peak.home - peak.away;

        cout << "Minute " << peak.minute
             << ": "
             << (net > 0 ? match.home : match.away)
             << " (net " << pts(net) << ")\n";
    }

    return report;
}

void comparePerformance(Session& s) {
    if (!s.tracker.loaded()) {
        cout << "Load the dataset first.\n";
        return;
    }

    cout << "\nRunning performance benchmark...\n";

    auto results = s.tracker.benchmark(
        s.opt.queries,
        s.opt.updates,
        s.opt.seed);

    filesystem::create_directories(s.opt.outDir);

    ofstream out(s.opt.outDir + "/benchmark.md");

    for (const auto& r : results) {
        cout << "\nEvents: " << r.n << "\n";

        cout << "Segment build: "
             << fmt(r.segBuildMs, 3) << " ms\n";
        cout << "Fenwick build: "
             << fmt(r.fenBuildMs, 3) << " ms\n";

        cout << "Segment query: "
             << fmt(r.segQueryNs, 2) << " ns\n";
        cout << "Fenwick query: "
             << fmt(r.fenQueryNs, 2) << " ns\n";
        cout << "Naive query: "
             << fmt(r.naiveQueryNs, 2) << " ns\n";

        cout << "Segment update: "
             << fmt(r.segUpdateNs, 2) << " ns\n";
        cout << "Fenwick update: "
             << fmt(r.fenUpdateNs, 2) << " ns\n";

        cout << "Segment memory: "
             << r.segBytes << " bytes\n";
        cout << "Fenwick memory: "
             << r.fenBytes << " bytes\n";

        cout << "Correctness: "
             << (r.allAgree ? "PASS" : "FAIL")
             << "\n";

        out << "## Events: " << r.n << "\n\n"
            << "Segment build: " << r.segBuildMs << " ms\n\n"
            << "Fenwick build: " << r.fenBuildMs << " ms\n\n"
            << "Segment query: " << r.segQueryNs << " ns\n\n"
            << "Fenwick query: " << r.fenQueryNs << " ns\n\n"
            << "Naive query: " << r.naiveQueryNs << " ns\n\n"
            << "Segment update: " << r.segUpdateNs << " ns\n\n"
            << "Fenwick update: " << r.fenUpdateNs << " ns\n\n"
            << "Verified: " << r.verified << "\n\n"
            << "Correct: " << (r.allAgree ? "yes" : "no")
            << "\n\n";
    }
}

void visualizeMomentum(Session& s) {
    if (!ready(s)) return;

    filesystem::create_directories(s.opt.outDir);

    const Match& match = s.tracker.matches()[s.match];

    ShiftReport report = s.tracker.detectShifts(
        s.match,
        s.opt.window,
        s.opt.threshold,
        preferredTree(s));

    string base = s.opt.outDir + "/momentum_" +
                  to_string(match.id);

    string csv = base + ".csv";
    string png = base + ".png";

    if (!s.tracker.exportSeriesCsv(s.match, report, csv)) {
        cout << "Unable to export CSV.\n";
        return;
    }

    string command =
        "python3 \"" + s.opt.plotScript + "\" \"" +
        csv + "\" \"" + png + "\" " +
        to_string(s.opt.window);

    int result = system(command.c_str());

    if (result == 0)
        cout << "Graph saved to " << png << "\n";
    else
        cout << "Plot failed. Check Python and matplotlib.\n";
}

int readInt(const string& prompt, int defaultValue) {
    cout << prompt << " [" << defaultValue << "]: ";

    string input;
    if (!getline(cin, input))
        return defaultValue;

    if (input.empty())
        return defaultValue;

    try {
        return stoi(input);
    } catch (...) {
        return defaultValue;
    }
}

void menu(Session& s) {
    while (true) {
        cout << "\n===== Soccer Momentum Tracker =====\n";
        cout << "1. Load Dataset\n";
        cout << "2. Select Match\n";
        cout << "3. Build Segment Tree\n";
        cout << "4. Build Fenwick Tree\n";
        cout << "5. Query Momentum Range\n";
        cout << "6. Detect Momentum Shifts\n";
        cout << "7. Compare Performance\n";
        cout << "8. Visualize Momentum\n";
        cout << "9. Exit\n";
        cout << "Choice: ";

        string input;
        if (!getline(cin, input))
            return;

        int choice;

        try {
            choice = stoi(input);
        } catch (...) {
            cout << "Invalid choice.\n";
            continue;
        }

        switch (choice) {
            case 1:
                loadDataset(s);
                break;

            case 2:
                if (!s.tracker.loaded()) {
                    cout << "Load the dataset first.\n";
                    break;
                }

                listMatches(s);
                selectMatch(s, readInt("Match ID or number", 1));
                break;

            case 3:
                if (s.tracker.loaded())
                    buildSegment(s);
                else
                    cout << "Load the dataset first.\n";
                break;

            case 4:
                if (s.tracker.loaded())
                    buildFenwick(s);
                else
                    cout << "Load the dataset first.\n";
                break;

            case 5:
                if (ready(s)) {
                    int from = readInt("From minute", 30);
                    int to = readInt("To minute", 45);
                    queryMomentum(s, from, to);
                }
                break;

            case 6:
                if (ready(s)) {
                    s.opt.window = readInt(
                        "Window size", s.opt.window);
                    detectMomentumShifts(s);
                }
                break;

            case 7:
                comparePerformance(s);
                break;

            case 8:
                visualizeMomentum(s);
                break;

            case 9:
                cout << "Goodbye!\n";
                return;

            default:
                cout << "Invalid choice.\n";
        }
    }
}

int main() {
    Session session;

    filesystem::create_directories(session.opt.outDir);

    menu(session);

    return 0;
}
