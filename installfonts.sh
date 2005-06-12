#/bin/bash
if [[ "$1" == "" ]]
    then
        echo "Command syntax is"
        echo "    installfont.sh <filename.ttf>"
        exit 0
fi

if [[ -d /usr/share/fonts/TTF ]]
    then 
        FontDir=/usr/share/fonts/TTF
    else
        if [[ -d /usr/X11R6/lib/X11/fonts/TTF ]]
            then 
                FontDir=/usr/X11R6/lib/X11/fonts/TTF
            else 
                echo "No TTF font directory found."
                exit -1
        fi
fi

echo Installing $@ to $FontDir
cp -v $@ $FontDir
mkfontscale $FontDir
mkfontdir $FontDir
