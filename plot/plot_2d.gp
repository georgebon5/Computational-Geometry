set terminal pngcairo size 1400,1400 enhanced font "Arial,12"
set output "results/convex_hull_2d.png"

set multiplot layout 2,2 \
    title "Κυρτό Περίβλημα — 150 τυχαία σημεία" font "Arial,15"

set xlabel "x"
set ylabel "y"
set grid lc rgb "#dddddd"
set key off

# ── Panel 1: Αυξητικός ──────────────────────────────────────────────────────
set title "Αυξητικός αλγόριθμος (Graham's Scan)" font "Arial,13"
plot "results/points_2d.dat"       with points pt 7 ps 0.7 lc rgb "#555555", \
     "results/hull_incremental.dat" with linespoints lw 2.5 lc rgb "#E53935" pt 7 ps 1.2

# ── Panel 2: Περιτυλίγματος ─────────────────────────────────────────────────
set title "Αλγόριθμος Περιτυλίγματος (Jarvis March)" font "Arial,13"
plot "results/points_2d.dat"  with points pt 7 ps 0.7 lc rgb "#555555", \
     "results/hull_gift.dat"  with linespoints lw 2.5 lc rgb "#1565C0" pt 7 ps 1.2

# ── Panel 3: Διαίρει και Βασίλευε ───────────────────────────────────────────
set title "Αλγόριθμος Διαίρει και Βασίλευε" font "Arial,13"
plot "results/points_2d.dat" with points pt 7 ps 0.7 lc rgb "#555555", \
     "results/hull_dc.dat"   with linespoints lw 2.5 lc rgb "#388E3C" pt 7 ps 1.2

# ── Panel 4: QuickHull ───────────────────────────────────────────────────────
set title "Αλγόριθμος QuickHull" font "Arial,13"
plot "results/points_2d.dat"      with points pt 7 ps 0.7 lc rgb "#555555", \
     "results/hull_quickhull.dat" with linespoints lw 2.5 lc rgb "#F57C00" pt 7 ps 1.2

unset multiplot
