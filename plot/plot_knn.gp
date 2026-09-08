set terminal pngcairo size 1000,900 enhanced font "Arial,12"
set output "results/knn.png"

set title "k-ΠΠΓ Ταξινόμηση (k=5, 300 σημεία εκπαίδευσης)\nΠεριοχές Απόφασης — kd-tree k-Nearest Neighbors" \
    font "Arial,14"
set xlabel "x" font "Arial,11"
set ylabel "y" font "Arial,11"
set xrange [-102:102]
set yrange [-102:102]
set size ratio 1
set grid lc rgb "#e8e8e8" lt 1
set key top right box lc rgb "#888888" samplen 2 font "Arial,10"
set border lw 1.5

# Layer 1: faint coloured regions = decision boundary (background)
# Layer 2: training points (solid circles, larger)
# Layer 3: query points (filled diamonds, matching predicted class)

plot \
  "results/knn_grid_c0.dat" with points pt 7 ps 0.55 lc rgb "#FFCDD2" notitle, \
  "results/knn_grid_c1.dat" with points pt 7 ps 0.55 lc rgb "#BBDEFB" notitle, \
  "results/knn_grid_c2.dat" with points pt 7 ps 0.55 lc rgb "#C8E6C9" notitle, \
  "results/knn_train_c0.dat" with points pt 7 ps 1.0 lc rgb "#B71C1C" title "Κλάση 0 — εκπαίδευση", \
  "results/knn_train_c1.dat" with points pt 7 ps 1.0 lc rgb "#0D47A1" title "Κλάση 1 — εκπαίδευση", \
  "results/knn_train_c2.dat" with points pt 7 ps 1.0 lc rgb "#1B5E20" title "Κλάση 2 — εκπαίδευση", \
  "results/knn_query_c0.dat" with points pt 13 ps 1.9 lw 2 lc rgb "#E53935" title "Ερώτημα → Κλ. 0", \
  "results/knn_query_c1.dat" with points pt 13 ps 1.9 lw 2 lc rgb "#1E88E5" title "Ερώτημα → Κλ. 1", \
  "results/knn_query_c2.dat" with points pt 13 ps 1.9 lw 2 lc rgb "#43A047" title "Ερώτημα → Κλ. 2"
