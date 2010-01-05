#/bin/bash
epydoc --html -vv -o reference --name libavg --exclude=avg_videoplayer --exclude=avg_showcamera --exclude=avg_showfont --exclude=avg_videoinfo --exclude=camcalibrator --url ../index.php --docformat epytext --no-frames --graph classtree --inheritance grouped --css epydoc_libavg.css --no-private --graph-font="lucida console" --debug libavg
