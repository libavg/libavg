use("avg.js");

function quitTimeout()
{
    print ("JS: timeout()");
    AVGPlayer.stop();
}

function tryLoadFile(fileName)
{
    var ok = AVGPlayer.loadFile(fileName);
    if (!ok) {
        print ("js: AVGPlayer.loadFile returned false");
    }
    return ok
}

function getVideoInfo(nodeName)
{
    var node = AVGPlayer.getElementByID(nodeName);
    print ("Number of frames in video: " + node.getNumFrames());
}

function videoPlay(nodeName)
{
    var node = AVGPlayer.getElementByID(nodeName);
    node.play();
}

function videoStop(nodeName)
{
    var node = AVGPlayer.getElementByID(nodeName);
    node.stop();
}

function videoPause(nodeName)
{
    var node = AVGPlayer.getElementByID(nodeName);
    node.pause();
}

AVGPlayer.setDebugOutput(AVGPlayer.DEBUG_WARNING |
                         AVGPlayer.DEBUG_CONFIG |
                         AVGPlayer.DEBUG_PROFILE); 
var ok = tryLoadFile("videofmt.avg");
if (ok) {
    getVideoInfo('mpeg1');
    videoPlay('mpeg1');
    AVGPlayer.setTimeout(20000, "videoStop('mpeg1');");

    AVGPlayer.setTimeout(10, "videoPlay('divx');");
    AVGPlayer.setTimeout(20000, "videoStop('divx');");
    AVGPlayer.setTimeout(20050, "quitTimeout();");

    AVGPlayer.play(25);
}

