#/bin/bash
cd reference
doxygen Doxyfile
cd -
man ../man/avg.1 | man2html -title avg avg > man_avg.html
man ../man/avgrc.5 | man2html -title avgrc avgrc > man_avgrc.html

