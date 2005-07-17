var Log = new Logger;
Log.setCategories(Log.LOG_PROFILE | 
                  Log.LOG_WARNING | 
                  Log.LOG_CONFIG);

useModule("player");
var AVGPlayer = new Player;

useModule("system");
var Sys = new System;

use ("anim.js");

var NUM_TEXTLINES = 15;

var Chapters = 
    [ {name: "title", numslides: 1},
      {name: "contents", numslides: 2},
      {name: "requirements", numslides: 6},
      {name: "concept", numslides: 3},
      {name: "basetech", numslides: 6},
      {name: "features", numslides: 12},
      {name: "samples", numslides: 6},
      {name: "future", numslides: 3},
      {name: "wishlist", numslides: 2}];

var Slides = 
    [ {},
      {},  // Contents title
      {title: "Introduction",
       text0: "- Plattform for multimedia installations",
       text1: "- Framework for media presentations",
       text2: "- High-quality visuals",
       text3: "- Alternative to Macromedia Director",
       text4: "- Open source (LGPL)" },
      {},  // Requirements title
      {title: "Quick Authoring",
       text0: "Authors need an easy and flexible method of specifying\
 the intended results. Short time intervals between\
 authoring changes and seeing\
 the results are essential."
      },
      {title: "High-quality Visuals",
       text0: "- Flexible engine",
       text1: "- Seamless media integration",
       text2: "- High-resolution video playback",
       text3: "- Good font rendering, layout support" },
      {title: "High-quality Sound",
       text0: "- CD-Quality, 44.1/48 kHz, 16 bit",
       text1: "- Low latency",
       text2: "- Mixer",
       text3: "- Nice to have: multi-channel playback"
      },
      {title: "Hardware/OS Support",
       text0: "- Linux:",
       text1: "  ° Low price",
       text2: "  ° 'Embedded' support",
       text3: "  ° Cloneable systems",
       text4: "  ° Legacy hardware support",
       text5: "- Mac OS X:",
       text6: "  ° Authoring tools"
      },
      {title: "Openness for Expansion",
       text0: "The potential users of the plattform are free-minded, intelligent\
 people who don't like to be fenced in. ",
       text3: "- Open source",
       text4: "- Plugin support"
      },
      {},  // Concept title
      {title: "Concept",
       text0: "- Separation of static and dynamic content",
       text1: "- xml-based screen layout language",
       text2: "  ° Based on svg",
       text3: "  ° Hierarchial",
       text4: "- Dynamic behaviour using JavaScript",
       text5: "  ° Animations",
       text6: "  ° User interaction"
      },
      {title: "Example",
       text0: '&lt;avg width="640" height="480"&gt;',
       text1: '  &lt;image id="logo" x="0" y="0"',
       text2: '         href="logo.tif"',
       text3: '         onmouseover="disappear();"/&gt;',
       text4: '&lt;/avg&gt;',
       text7: 'function disappear() {',
       text8: '    var logoNode =',
       text9: '       AVGPlayer.getElementByID',
       text10: '                     ("logo");',
       text11: '    logoNode.opacity = 0;',
       text12:'}'
      },
      {},  // Base Technologies Title
      {title: "Screen Output",
       text0: "- OpenGL, DirectFB",
       text1: "- Hardware-based rendering",
       text2: "- OpenGL:",
       text3: "  ° 3d",
       text4: "  ° Good OS support",
       text5: "  ° (Comparatively) good driver support",
       text6: "- DirectFB",
       text7: "  ° Efficient 2d rendering",
       text8: "  ° Linux only",
       text9: "  ° Good on low-end systems"
      },
      {title: "Image Support",
       text0: "- paintlib",
       text1: "- Support for 15 image formats",
       text2: "- Alternatives: imagemagick/graphicsmagick"
      },
      {title: "Video Support",
       text0: "- libavcodec:",
       text1: "  ° Heart of ffmpeg",
       text2: "  ° Masses of video formats",
       text3: "  ° External video sources",
       text4: "- libdc1394:",
       text5: "  ° Industrial cameras",
       text6: "  ° Firewire interface"
      },
      {title: "Text Output",
       text0: "- libfreetype:",
       text1: "  ° Font engine",
       text2: "  ° Renders individual characters",
       text3: "- Pango:",
       text4: "  ° Text layout engine",
       text5: "  ° Used in gnome",
       text6: "  ° Supports strange character sets",
       text7: "  ° Supports right-to-left-rendering"
      },
      {title: "Backend",
       text0: "- libxml2",
       text1: "- Spidermonkey"
      },
      {},  // Features Title
      {title: "Display Mixer",
       text0: "- Central to avg",
       text1: "- Generic handling of graphical elements",
       text2: "- Backend: OpenGL or DirectFB",
       text3: "- Scale &amp; crop support",
       text4: "- Alpha, opacity support"
      },
      {title: "Display Mixer - OpenGL Backend",
       text0: "- Rendering in hardware",
       text1: "- Rotation",
       text2: "- Subpixel precision",
       text3: "- Additive &amp; subtractive blend modes",
       text4: "- Warping",
       text5: "- Everything adjustable in real time"
      },
      {title: "Display Mixer - DirectFB Backend",
       text0: "- Software rendering",
       text1: "- Works on low-end hardware"
      },
      {title: "Video Support",
       text0: "- Several videos at once",
       text1: "- play/pause/stop at any time using javascript",
       text2: "- seek support",
       text3: "- Framerate synchronized with avg framerate",
       text4: "- Complete integration",
       text5: "- Speed: P4 Mobile 1.6 GHz shows 5 videos",
       text6: "  ° 352x240 resolution",
       text7: "  ° 25 fps"
      },
      {},    // Video example
      {title: "Text Support",
       text0: "- Pangos &lt;div&gt; tag support in avg files.",
       text1: "- Bold, italic, color,...",
       text2: "- Left/right-justified, centered paragraphs",
       text3: "- High-quality font rendering",
       text4: "- Utf8-support",
       text5: "- Fully integrated"
      },
      {},    // Text example
      {title: "Camera Support",
       text0: "- Board cameras using firewire",
       text1: "- Setting camera params supported",
       text2: "  ° brightness",
       text3: "  ° exposure",
       text4: "  ° saturation",
       text5: "  ° gamma",
       text6: "  ° ...",
       text7: "- Example? Kaputt"
      },
      {title: "Input Support",
       text0: "- Mouse/touchscreen:",
       text1: "  ° JavaScript handlers for display elements",
       text2: "  ° MouseDown/Up/Over/Out/Move",
       text3: "  ° Mouse event bubbling",
       text4: "- Keyboard",
       text5: "- Parallel port control lines",
       text6: "- Hack: Soundcard as A/D converter"
      },
      {title: "Authoring",
       text0: "- Complete JavaScript language is available",
       text1: "- Display element properties as js variables:",
       text2: "      (Example: image.angle += 3.14159)",
       text3: "- React to user events (preceeding section)",
       text4: "- Timers (setTimeout, setInterval)",
       text5: "- System call support",
       text6: "- Print to console support",
       text7: "- Javascript base libraries"
      },
      {title: "Other Stuff",
       text0: "- Strange conrad relais cards",
       text1: "- Plugin support (dlopen)",
       text2: "- Profiling",
       text3: "- High-precision frame timing (from mplayer)",
       text4: "- Panorama image support"
      },
      {},  // Samples title
      {title: "timescope"
      },
      {title: "timescope"
      },
      {title: "c-base scanner"
      },
      {title: "c-base scanner"
      },
      {title: "c-base scanner"
      },
      {},  // Future title
      {title: "More",
       text0: "- Better documentation",
       text1: "- Better mac support",
       text2: "- Sound",
       text3: "- Dynamic adding/deleting of scene elements",
       text4: "- Layer compositing operators (HLSL)",
       text5: "- Generic shader support"
      },
      {title: "Even more",
       text0: "- Better memory management",
       text1: "- Networking",
       text2: "- Stylesheet support?",
       text3: "- More media types:",
       text4: "  ° svg?",
       text5: "  ° Quicktime on mac?",
       text6: "  ° Video4Linux"
      },
      {},   // Wishlist title
      {title: "Wishlist",
       text0: "- Participation:",
       text1: "  ° Developers",
       text2: "  ° Artists/Designers",
       text3: "  ° http://www.libavg.de",
       text4: "  ° Mailing list: lists.sourceforge.net/",
       text5: "           lists/listinfo/avg-user",
       text6: "- No more software patent issues.",
       text8: "Finding me: look in art+beauty room"
      }
    ]

var curChapter = -1;
var curSlide = -1;
var numSlides = 0;

function init() {
    for (i in Chapters) {
        var curChapter = Chapters[i];
        if (i==0) {
            Chapters[i].startslide = 0;
        } else {
            Chapters[i].startslide = 
                    Chapters[i-1].startslide+Chapters[i-1].numslides;
        }
        numSlides += Chapters[i].numslides;
    }

    for (i=0; i<NUM_TEXTLINES; i++) {
        var curTextNode = AVGPlayer.getElementByID("slide_text"+i);
        curTextNode.y = 260+i*39;
        curTextNode.parawidth = 500;
    }
}

function videoPlay(nodeName)
{
    var node = AVGPlayer.getElementByID(nodeName);
    print ("Starting "+nodeName);
    node.play();
    fadeIn(nodeName, 200, 0.8);
}

function videoStop(nodeName)
{
    var node = AVGPlayer.getElementByID(nodeName);
    node.stop();
    node.opacity = 0;
}

function rotateVideos()
{
    var i;
    for (i=1; i<7; ++i) {
        var node = AVGPlayer.getElementByID("mpeg"+i);
        node.angle += 0.01;
    }
}

var rotateInterval;

function showVideos() {
    fadeIn("videosample", 2000, 1.0);
    fadeOut("mainscreen", 2000);
    rotateInterval = AVGPlayer.setInterval(10, "rotateVideos()");
    timeout1 = AVGPlayer.setTimeout(10, "videoPlay('mpeg1');");
    timeout2 = AVGPlayer.setTimeout(4000, "videoPlay('mpeg2');");
    timeout3 = AVGPlayer.setTimeout(8000, "videoPlay('mpeg3');");
    timeout4 = AVGPlayer.setTimeout(12000, "videoPlay('mpeg4');");
    timeout5 = AVGPlayer.setTimeout(16000, "videoPlay('mpeg5');");
//    timeout6 = AVGPlayer.setTimeout(20000, "videoPlay('mpeg6');");
}

function hideVideos() {
    fadeOut("videosample", 1000);
    fadeIn("mainscreen", 1000, 1.0);
    AVGPlayer.setTimeout(1200, "AVGPlayer.clearInterval(rotateInterval);");
    AVGPlayer.clearInterval(timeout1);
    AVGPlayer.clearInterval(timeout2);
    AVGPlayer.clearInterval(timeout3);
    AVGPlayer.clearInterval(timeout4);
    AVGPlayer.clearInterval(timeout5);
    AVGPlayer.setTimeout(1200, "for (i=1; i<7; ++i) { videoStop('mpeg'+i); };");
}

function showLayout() {
    fadeIn("layoutsample", 1000, 1.0);
}

function hideLayout() {
    fadeOut("layoutsample", 500);
}

function showTimescope() {
    videoPlay("timescope");
    fadeIn("timescope", 300, 1.0);
}

function hideTimescope() {
    videoStop("timescope");
    fadeOut("timescope", 300);
}

function getChapterFromSlide(slide) {
  for (i in Chapters) {
      if (slide < Chapters[i].startslide) {
          return i-1;
      }
  }
  return Chapters.length-1;
}

function setConsoleFont(bOk) {
    var fontName;
    var fontSize;
    if (bOk) {
        fontName = "courier";
        fontSize = "22";
        lineSpacing = "25";
    } else {
        fontName = "eurostile";
        fontSize = "28";
        lineSpacing = "37";
    }
    for (var i=0; i<NUM_TEXTLINES; i++) {
        var curNode = AVGPlayer.getElementByID("slide_text"+i);
        curNode.font = fontName;
        curNode.size = fontSize;
        curNode.y = 260+i*lineSpacing;
    }
}

function showDefaultSlide(slide) {
    setConsoleFont(false);
    var curSlideText = Slides[slide];
    if (curSlideText) {
        if (curSlideText.title) {
            AVGPlayer.getElementByID("slide_title").text = curSlideText.title;
        } else {
            AVGPlayer.getElementByID("slide_title").text = "";
        }
        for (i=0; i<NUM_TEXTLINES; i++) {
            var curText = eval("curSlideText.text"+i);
            if (curText) {
                AVGPlayer.getElementByID("slide_text"+i).text = curText;
            } else {
                AVGPlayer.getElementByID("slide_text"+i).text = "";
            }
        }
    } else {
        AVGPlayer.getElementByID("slide_title").text = "";
        for (i=0; i<NUM_TEXTLINES; i++) {
            AVGPlayer.getElementByID("slide_text"+i).text = "";
        }
    }
}

function switchToSlide(slide) {
    var lastSlide = curSlide;
    switch (lastSlide) {
        case 23:
            hideVideos();
            break;
        case 25:
            hideLayout();
            break;
        case 31:
            fadeOut("timescope_rendering", 300);
            break;
        case 32:
            hideTimescope();
            break;
        case 33:
            fadeOut("airlock1", 300);
            break;
        case 34:
            fadeOut("airlock2", 300);
            break;
        case 35:
            fadeOut("airlock3", 300);
            break;
    }
    var headings = AVGPlayer.getElementByID("headings");
    if (slide < 0) {
        slide = 0;
    }
    if (slide >= numSlides) {
        slide = numSlides-1;
    }
    curSlide = slide;
    var lastChapter = curChapter;
    curChapter = getChapterFromSlide(curSlide);
    print ("curChapter: "+curChapter+", lastChapter: "+lastChapter+
           ", curSlide: "+curSlide+", lastSlide: "+lastSlide);
    var curChapterName = Chapters[curChapter].name;
    if (curChapter != lastChapter) {
        // Hide all headings first
        for (i=0; i<headings.getNumChildren(); i++) {
            headings.getChild(i).opacity = 0.3;
        }
        AVGPlayer.getElementByID(curChapterName+"_heading").opacity=1.0;
    }
    var curImage = "";
    if (curSlide == 0 || curChapter != getChapterFromSlide(curSlide-1)) {
        curImage = curChapterName;
        fadeIn (curChapterName, 300, 1.0);
    }
    var images = AVGPlayer.getElementByID("images");
    for (i=0; i<images.getNumChildren(); i++) {
        var id = images.getChild(i).id;
        if (images.getChild(i).opacity != 0 && id != curImage) {
           fadeOut(id, 300);
        }
    }
    var logo = AVGPlayer.getElementByID("logo");
    if (curSlide == 0) {
        logo.opacity = 0;
    } else {
        if (logo.opacity != 1) {
            fadeIn("logo", 300, 1.0);
        }
    }
    showDefaultSlide(curSlide);
    switch (curSlide) {
        case 11:
            setConsoleFont(true);
            break;
        case 23:
            showVideos();
            break;
        case 25:
            showLayout();
            break;
        case 31:
            fadeIn("timescope_rendering", 300, 1.0);
            break;
        case 32:
            showTimescope();
            break;
        case 33:
            fadeIn("airlock1", 300, 1.0);
            break;
        case 34:
            fadeIn("airlock2", 300, 1.0);
            break;
        case 35:
            fadeIn("airlock3", 300, 1.0);
            break;
    }
}

var ShotNum = 0;

function onKeyUp() {
    var Event = AVGPlayer.getCurEvent();
    switch (Event.keystring) {
        case 'page up':
            switchToSlide(curSlide-1);
            break;
        case 'page down':
            switchToSlide(curSlide+1);
            break;   
        case 'print screen':
            ShotNum++;
            AVGPlayer.screenshot("avg_presentation"+ShotNum+".png");
            break;
        case 'down':
            break;
        default:
            print(Event.keystring);
            return;
    } 
}

AVGPlayer.loadFile("lecture.avg");
// AVGPlayer.setInterval(10,"onFrame();");
init();
switchToSlide(0);

AVGPlayer.showCursor(false);
AVGPlayer.play(25);
