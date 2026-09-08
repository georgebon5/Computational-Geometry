set terminal pngcairo size 950,900 enhanced font "Arial,12"
set output "results/search_result.png"

set title "Ορθογώνια Γεωμετρική Αναζήτηση με kd-tree\n150 τυχαία σημεία,  ερώτημα: x∈[-30,60] × y∈[-20,70]" \
    font "Arial,13"
set xlabel "x" font "Arial,11"
set ylabel "y" font "Arial,11"
set grid lc rgb "#eeeeee" lt 1
set xrange [-110:110]
set yrange [-110:110]
set key top right box lc rgb "#888888"
set border lw 1.5

plot "results/search_splits_x.dat"  with lines lw 0.7 lc rgb "#ffcccc" \
         notitle, \
     "results/search_splits_y.dat"  with lines lw 0.7 lc rgb "#cce0ff" \
         notitle, \
     "results/search_all_pts.dat"   with points pt 7 ps 0.9 lc rgb "#aaaaaa" \
         title "Σύνολο σημείων (150)", \
     "results/search_query.dat"     with lines lw 2.5 lc rgb "#1565C0" dt 2 \
         title "Ορθογώνιο ερωτήματος", \
     "results/search_found_pts.dat" with points pt 7 ps 1.6 lc rgb "#E53935" \
         title "Σημεία εντός ορθογωνίου"
