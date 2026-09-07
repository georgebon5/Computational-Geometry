set terminal pngcairo size 900,800 enhanced font "Arial,12"
set output "results/lp.png"

set title "Γραμμικός Προγραμματισμός — Αλγόριθμος Seidel\nmax{-5x_1+12x_2},  x*=(127/7, 43/7) {/Symbol \273} (18.14, 6.14),  obj*=-17" \
    font "Arial,13"
set xlabel "x_1" font "Arial,11"
set ylabel "x_2" font "Arial,11"
set grid lc rgb "#dddddd" lt 1 lw 0.7
set xrange [12:32]
set yrange [2:11]
set key top right box lc rgb "#888888"
set border lw 1.5

set style fill transparent solid 0.35 noborder

plot "results/lp_region.dat"      with filledcurves closed \
         lc rgb "#A5D6A7" title "Εφικτή περιοχή", \
     "results/lp_region.dat"      with lines lw 2.5 lc rgb "#2E7D32" \
         notitle, \
     "results/lp_constraints.dat" with lines lw 1.5 lc rgb "#1565C0" dt 2 \
         title "Περιορισμοί (σύνορα)", \
     "results/lp_objectives.dat"  with lines lw 1.5 lc rgb "#E53935" dt 3 \
         title "Ισοϋψείς αντικ. συνάρτ.", \
     "results/lp_optimal.dat"     with points pt 3 ps 3.5 lw 2.5 \
         lc rgb "#F57C00" title "Βέλτιστο x*"
