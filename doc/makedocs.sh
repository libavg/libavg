#/bin/bash
epydoc --html -o reference -n libavg -u ../index.html --no-private --docformat plaintext --no-frames  /usr/local/lib/python2.4/site-packages/libavg/avg.so
cp ../avg.dtd ./avg_dtd.txt
#man ../man/avg.1 | man2html -title avg avg > man_avg.html
#man ../man/avgrc.5 | man2html -title avgrc avgrc > man_avgrc.html

