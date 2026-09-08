set terminal pngcairo size 950,700 enhanced font "Arial,12"
set output "results/knn_timing.png"

set title "k-NN: kd-tree vs Γραμμική Σάρωση\n(k=5, 100 ερωτήματα, μέσος όρος 3 επαναλήψεων)" \
    font "Arial,13"
set xlabel "Πλήθος σημείων εκπαίδευσης N" font "Arial,11"
set ylabel "Χρόνος (ms)" font "Arial,11"
set logscale x
set logscale y
set format x "10^{%T}"
set grid lc rgb "#dddddd" lt 1
set key top left box lc rgb "#888888"
set border lw 1.5

plot \
  "results/knn_timing.dat" using 1:2 \
      with linespoints pt 7 ps 1.4 lw 2 lc rgb "#7B1FA2" \
      title "Κατασκευή kd-tree  O(N log N)", \
  "results/knn_timing.dat" using 1:3 \
      with linespoints pt 5 ps 1.4 lw 2 lc rgb "#1565C0" \
      title "k-NN με kd-tree  O(k log N)", \
  "results/knn_timing.dat" using 1:4 \
      with linespoints pt 9 ps 1.4 lw 2 lc rgb "#C62828" \
      title "Γραμμική σάρωση  O(N)"
