use("avg.js");

function tryLoadFile(fileName)
{
    var ok = AVGPlayer.loadFile(fileName);
    if (!ok) {
        print ("js: AVGPlayer.loadFile returned false");
    }
    return ok
}

function play()
{
    var node = AVGPlayer.getElementByID("camera");
    node.play();
}

function stop()
{
    var node = AVGPlayer.getElementByID("camera");
    node.stop();
}

function pause()
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
    AVGPlayer.play(10000);
}

