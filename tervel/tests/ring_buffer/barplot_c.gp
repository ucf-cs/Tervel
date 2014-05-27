oFile=file.".pdf";
set datafile separator "\t"
set terminal pdf;

fontSize="6"

set key under center font "Times-Roman, ".fontSize
set xlabel "Threads" font "Times-Roman, ".fontSize
set format y "%.2e"

set ylabel "Operations Completed" font "Times-Roman, ".fontSize

set xtics font "Times-Roman, ".fontSize
set ytics font "Times-Roman, ".fontSize

set   autoscale
set output oFile

set style fill solid
set key autotitle columnhead
set style data histogram
set style histogram cluster gap 1

plot file u 2:xticlabels(1),\
     file u 3:xticlabels(1),\
     file u 4:xticlabels(1),\
     file u 5:xticlabels(1),\
     file u 6:xticlabels(1),\
     file u 7:xticlabels(1)


unset output
