use("avg.js");

function tryLoadFile(fileName)
{
    var ok = AVGPlayer.loadFile(fileName);
    if (!ok) {
        print ("js: AVGPlayer.loadFile returned false");
    }
    return ok;
}

function pan()
{
    var node = AVGPlayer.getElementByID("pano");
    node.rotation+=0.1;
    if (node.rotation > node.getMaxRotation()) {
        node.rotation = 0;
    }
}

AVGPlayer.setDebugOutput(AVGPlayer.DEBUG_WARNING |
                         AVGPlayer.DEBUG_CONFIG |
                         AVGPlayer.DEBUG_PROFILE); 
var ok = tryLoadFile("panoimage.avg");
if (ok) {
    AVGPlayer.setInterval(10, "pan();");
    AVGPlayer.play(15);
}

