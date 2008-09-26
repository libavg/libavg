#/bin/bash
epydoc --html -vv -o reference --name libavg --exclude=avg_videoplayer --url ../index.php --docformat epytext --no-frames --graph classtree --inheritance grouped --css epydoc_libavg.css --no-private --graph-font="lucida console" libavg
