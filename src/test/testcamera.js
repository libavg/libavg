use("avg.js");

function tryLoadFile(fileName)
{
    var ok = AVGPlayer.loadFile(fileName);
    if (!ok) {
        print ("js: AVGPlayer.loadFile returned false");
    }
    return ok
}

function play(nodeName)
{
    var node = AVGPlayer.getElementByID("camera");
    node.play();
}

function stop(nodeName)
{
    var node = AVGPlayer.getElementByID("camera");
    node.stop();
}

function pause(nodeName)
{
    var node = AVGPlayer.getElementByID("camera");
    node.pause();
}

AVGPlayer.setDebugOutput(AVGPlayer.DEBUG_WARNING |
                         AVGPlayer.DEBUG_CONFIG |
                         AVGPlayer.DEBUG_PROFILE); 
var ok = tryLoadFile("camera.avg");
if (ok) {
    play();
//    AVGPlayer.setTimeout(20000, "stop();");
    AVGPlayer.play(25);
}

