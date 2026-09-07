set terminal pngcairo size 950,620 enhanced font "Arial,12"
set output "results/timing.png"

set title "Σύγκριση χρόνων εκτέλεσης αλγορίθμων κυρτού περιβλήματος" font "Arial,13"
set xlabel "Πλήθος σημείων n" font "Arial,11"
set ylabel "Μέσος χρόνος εκτέλεσης (ms)" font "Arial,11"
set logscale x
set logscale y
set format x "10^{%L}"
set grid lc rgb "#dddddd" lt 1
set key top left box lc rgb "#888888"
set border lw 1.5

set style data linespoints

plot "results/timing.dat" \
     using 1:2 with linespoints pt 7  ps 1.3 lw 2.2 lc rgb "#1976D2" \
         title "Αυξητικός O(n log n)", \
     ""        using 1:3 with linespoints pt 5  ps 1.3 lw 2.2 lc rgb "#E53935" \
         title "Περιτύλιγμα O(nh)", \
     ""        using 1:4 with linespoints pt 9  ps 1.3 lw 2.2 lc rgb "#388E3C" \
         title "Διαίρει & Βασίλευε O(n log n)", \
     ""        using 1:5 with linespoints pt 13 ps 1.3 lw 2.2 lc rgb "#F57C00" \
         title "QuickHull O(n log n) avg"
