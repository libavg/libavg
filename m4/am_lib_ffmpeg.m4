# Cool ffmpeg check taken from GplFlash :-)
AC_DEFUN([AM_LIB_FFMPEG], [
    #Test for ffmpeg
    #See if ffmpeg-config is available...
    AC_MSG_CHECKING([[if ffmpeg-config is present]])
    if ffmpeg-config --version 1>/dev/null 2>/dev/null; then
        AC_MSG_RESULT([[yes]])
        LIBFFMPEG="`ffmpeg-config --plugin-libs avcodec avformat`"
    else
        AC_MSG_RESULT([[no]])
    fi

    # If ffmpeg-config didn't work, see if ffmpeg is registered with pkg-config
    if test "x$LIBFFMPEG" == "x"; then
        PKG_CHECK_MODULES([FFMPEG], [libavcodec libavformat], [LIBFFMPEG="$FFMPEG_LIBS"], [:])
    fi
    
    # Otherwise, try a plain-jane linking and header search.
    if test "x$LIBFFMPEG" == "x"; then
        AC_CHECK_LIB([avformat_pic], [av_register_all],
                     [LIBFFMPEG="-lavformat_pic -lavcodec_pic"],
                     [AC_CHECK_LIB([avformat], [av_register_all],
                                   [LIBFFMPEG="-lavformat -lavcodec"],
                                   [AC_MSG_ERROR([[libavg Requires libavcodec and libavformat (ffmpeg)]])],
                                   [-lavcodec]
                                   )],
                     [-lavcodec_pic]
                     )
        
        if test "x$LIBFFMPEG" == "x"; then
            AC_CHECK_HEADER([ffmpeg/avformat.h], [],
                            [AC_MSG_ERROR([[ffmpeg header files not found.]])])
        fi
    fi

    AC_SUBST(LIBFFMPEG)
])
