use("avg.js");

function tryLoadFile(fileName)
{
    var ok = AVGPlayer.loadFile(fileName);
    if (!ok) {
        print ("js: AVGPlayer.loadFile returned false");
    }
    return ok;
}


AVGPlayer.setDebugOutput(AVGPlayer.DEBUG_WARNING |
                         AVGPlayer.DEBUG_CONFIG |
                         AVGPlayer.DEBUG_PROFILE); 
var ok = tryLoadFile("panoimage.avg");
if (ok) {
    AVGPlayer.play(25);
}

