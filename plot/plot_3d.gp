set terminal pngcairo size 1000,900 enhanced font "Arial,12"
set output "results/convex_hull_3d.png"

set title "3D Κυρτό Περίβλημα — 150 τυχαία σημεία\n(Αυξητικός Beneath-Beyond αλγόριθμος, 68 έδρες)" \
    font "Arial,13"
set xlabel "x" font "Arial,11"
set ylabel "y" font "Arial,11"
set zlabel "z" font "Arial,11"
set ticslevel 0
set key off
set border lw 1.5
set view 55, 35, 1.1, 1.0

splot "results/points_3d.dat" \
          with points pt 7 ps 1.0 lc rgb "#1565C0" title "Σημεία", \
      "results/faces_3d.dat"  \
          with lines lw 1.2 lc rgb "#E53935" title "Έδρες"
