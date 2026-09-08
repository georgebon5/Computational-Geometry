set terminal pngcairo size 900,800 enhanced font "Arial,12"
set output "results/kdtree_construction.png"

set title "Κατασκευή kd-tree — 12 επιλεγμένα σημεία\n(Βέλτιστη κατασκευή με nth_element, O(n log n))" \
    font "Arial,13"
set xlabel "x" font "Arial,11"
set ylabel "y" font "Arial,11"
set grid lc rgb "#eeeeee" lt 1
set xrange [0:10]
set yrange [0:10]
set key top right box lc rgb "#888888"
set border lw 1.5

plot "results/kdtree_small_splits_x.dat" with lines lw 2.2 lc rgb "#E53935" \
         title "Τομή κατά x (βάθος άρτιο)",  \
     "results/kdtree_small_splits_y.dat" with lines lw 2.2 lc rgb "#1565C0" \
         title "Τομή κατά y (βάθος περιττό)", \
     "results/kdtree_small_pts.dat"      with points pt 7 ps 2.0 lc rgb "#212121" \
         title "Σημεία (12)"
