#/bin/bash
cd reference
doxygen Doxyfile
cd -
man ../man/avg.1 | man2html > man_avg.html
man ../man/avgrc.5 | man2html > man_avgrc.html

