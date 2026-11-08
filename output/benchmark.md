| n (events) | Seg build (ms) | BIT build (ms) | Seg query (ns) | BIT query (ns) | Naive query (ns) | Seg update (ns) | BIT update (ns) | Seg memory | BIT memory | Verified vs naive |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|:---|
| 10000 | 0.013 | 0.014 | 79.9 | 20.0 | 908 | 10.2 | 14.5 | 156.3 KiB | 78.2 KiB | 4000 queries OK |
| 100000 | 0.225 | 0.181 | 114.9 | 27.5 | 9747 | 13.6 | 15.9 | 1562.5 KiB | 781.3 KiB | 4000 queries OK |
| 462462 | 1.014 | 0.860 | 135.7 | 32.3 | 50225 | 24.9 | 27.4 | 7226.0 KiB | 3613.0 KiB | 4000 queries OK |
