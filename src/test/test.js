use("avg.js");

function quitTimeout()
{
    print ("JS: timeout()");
    AVGPlayer.stop();
}

function interval()
{
    dump (".");
}

function onMouseMove()
{
//    print ("JS: onMouseMove()");
}

function onMouseOver()
{
    print ("JS: onMouseOver()");
}
function onMouseOut()
{
    print ("JS: onMouseOut()");
}

function mainMouseUp()
{
    print ("JS: mainMouseUp()");
}

function mainMouseDown()
{
    print ("JS: mainMouseDown()");
}

function onMouseUp()
{
    var Event = AVGPlayer.getCurEvent();
    print ("JS: Event (type= "+Event.type+", pos=("+Event.xPosition+
           ", "+Event.yPosition+"), state="+Event.button+")");
    var Node = Event.getElement();
    print ("    Node: "+Node);
}

var timerid;

function tryLoadFile(fileName)
{
    var ok = AVGPlayer.loadFile(fileName);
    if (!ok) {
        print ("js: AVGPlayer.loadFile returned false");
    }
    return ok
}

function playAVG (fileName) 
{

    print ("---- Playing "+fileName+"----");
    AVGPlayer.setDebugOutput(AVGPlayer.DEBUG_PROFILE 
            | AVGPlayer.DEBUG_MEMORY | AVGPlayer.DEBUG_EVENTS2 | AVGPlayer.DEBUG_EVENTS);
    var ok = tryLoadFile(fileName);
    if (ok) {
        timerid = AVGPlayer.setInterval(40, "interval();");
        AVGPlayer.setTimeout(5000, "quitTimeout();");
        AVGPlayer.setTimeout(500, "AVGPlayer.clearInterval(timerid);");
//        AVGPlayer.setTimeout(1000, "AVGPlayer.setDebugOutput(AVGPlayer.DEBUG_EVENTS | AVGPlayer.DEBUG_PROFILE);");
//        AVGPlayer.setTimeout(2000, "AVGPlayer.setDebugOutput(AVGPlayer.DEBUG_PROFILE);");
        AVGPlayer.play(30);
    }
}

var delay = 0;
var NumChars = 0;

function textInterval()
{
    var node = AVGPlayer.getElementByID("cbasetext");
    delay++;
    if (delay == 10) {
      NumChars++;
      delay = 0;
    }
    var str = "hello c-base".substr(0, NumChars);
    node.text = str;
    node.x++;
}

function changeTextHeight()
{
    var node = AVGPlayer.getElementByID("cbasetext");
    node.height = 50;

    var l = node.x;
    var t = node.y;
    var w = node.width;
    var h = node.height;
    print ("Pos: ("+l+","+t+","+w+","+h+")\n");
}

function changeColor()
{
    var node = AVGPlayer.getElementByID("cbasetext");
    node.color = "404080";
}

function changeFont()
{
    var node = AVGPlayer.getElementByID("cbasetext");
    node.font = "lucon";
    node.size = 50;
}

function changeFont2()
{
    var node = AVGPlayer.getElementByID("cbasetext");
    node.size = 30;
}

function testWords()
{
    print ("---- Testing word node accessors ----");
    var ok = tryLoadFile("text.avg");
    if (ok) {
        AVGPlayer.setDebugOutput(AVGPlayer.DEBUG_BLTS | AVGPlayer.DEBUG_PROFILE | AVGPlayer.DEBUG_MEMORY); 
//        AVGPlayer.setDebugOutput(AVGPlayer.DEBUG_PROFILE | AVGPlayer.DEBUG_MEMORY); 
        timerid = AVGPlayer.setInterval(10, "textInterval();");
        AVGPlayer.setTimeout(1000, "changeTextHeight();");
        AVGPlayer.setTimeout(2000, "changeColor();");
        AVGPlayer.setTimeout(3000, "changeFont();");
        AVGPlayer.setTimeout(4000, "changeFont2();");
        AVGPlayer.setTimeout(8000, "quitTimeout();");

        AVGPlayer.play(25);
    }
}

function videoInterval()
{
    var node = AVGPlayer.getElementByID("clogo");
    node.x++;

}

function getVideoInfo()
{
    var node = AVGPlayer.getElementByID("clogo");
    print ("Number of frames in video: " + node.getNumFrames());
}

function videoPlay(nodeName)
{
    print(nodeName);
    var node = AVGPlayer.getElementByID(nodeName);
    node.play();
}

function videoStop()
{
    var node = AVGPlayer.getElementByID("clogo");
    node.stop();
}

function videoPause()
{
    var node = AVGPlayer.getElementByID("clogo");
    node.pause();
}

function videoReset(nodeName)
{
    var node = AVGPlayer.getElementByID(nodeName);
    print ("Current frame: " + node.getCurFrame());
    print ("FPS: " + node.getFPS());
/*    node.seekToFrame(300);*/
}

var cropInterval;

function cropTL()
{
    var node = AVGPlayer.getElementByID("img");
    node.x -= 2;
    node.y -= 2;
    if (node.x < -250) {
        AVGPlayer.clearInterval(cropInterval);
        cropInterval = AVGPlayer.setInterval(10, "cropBR();");
    }
}

function cropBR()
{
    var node = AVGPlayer.getElementByID("img");
    if (node.x < 0) {
        node.x = 100;
        node.y = 50;
    }
    node.x +=2;
    node.y +=2;
    if (node.x > 700) {
        AVGPlayer.clearInterval(cropInterval);
        AVGPlayer.stop();
    }
}

function goneTL()
{
    var node = AVGPlayer.getElementByID("img");
    node.x = -250;
    node.y = -250;
}

function goneBR()
{
    var node = AVGPlayer.getElementByID("img");
    node.x = 750;
    node.y = 650;
}

function testCrop()
{
    print ("---- Testing cropping ----");
    AVGPlayer.setDebugOutput(AVGPlayer.DEBUG_BLTS); 
    var ok = tryLoadFile("crop.avg");
    if (ok) {
        cropInterval = AVGPlayer.setInterval(10, "cropTL();");
        AVGPlayer.play(25);    
    }
}

function testVideo()
{
    print ("---- Testing video node accessors ----");
    var ok = tryLoadFile("video.avg");
    if (ok) {
//    AVGPlayer.setDebugOutput(AVGPlayer.DEBUG_BLTS); 
        AVGPlayer.setDebugOutput(AVGPlayer.DEBUG_PROFILE); 
        getVideoInfo();
        timerid = AVGPlayer.setInterval(10, "videoInterval();");
        videoPlay('clogo');
        AVGPlayer.setTimeout(1500, "videoPlay('clogo1');");
        AVGPlayer.setTimeout(1000, "videoPlay('clogo2');");
        AVGPlayer.setTimeout(2000, "videoPause();");
        AVGPlayer.setTimeout(2500, "videoPlay('clogo');");
        AVGPlayer.setTimeout(3000, "videoStop();");
        AVGPlayer.setTimeout(3500, "videoPlay('clogo');");
        AVGPlayer.setTimeout(3700, "videoReset('clogo');");
        AVGPlayer.setTimeout(4000, "videoReset('clogo');");
        AVGPlayer.setTimeout(4300, "videoReset('clogo');");
        AVGPlayer.setTimeout(4600, "videoReset('clogo');");
        AVGPlayer.setTimeout(4900, "videoReset('clogo');");
        AVGPlayer.setTimeout(5200, "videoReset('clogo');");
        AVGPlayer.setTimeout(8000, "quitTimeout();");
        AVGPlayer.play(25);
    }
}

function dumpNodes()
{
    print ("---- dumpNodes: testing node accessors ----");
    var ok = tryLoadFile("avg.avg");
    if (ok) {
        var rootNode = AVGPlayer.getRootNode();
        var numChildren = rootNode.getNumChildren();
        print("  Root node id: "+rootNode.getID()+" ("+numChildren+" children).");

        var i;
        for (i=0; i<numChildren; i++) {
            var curChild = rootNode.getChild(i);
            var parent = curChild.getParent();
            var l = curChild.x;
            var t = curChild.y;
            var w = curChild.width;
            var h = curChild.height;
            print("    Child node id: "+curChild.getID()+" (Parent: "+parent.getID()
                    +", Pos: ("+l+","+t+","+w+","+h+"), type: "+curChild.getType()+")");
        }
        AVGPlayer.setTimeout(100, "quitTimeout()");
        AVGPlayer.play(30);
    }
}

function moveit() {
    var node = AVGPlayer.getElementByID("nestedimg1");
    node.x++;
    node.opacity -= 0.01;
}

function testAnimation()
{
    print ("---- testing Animation  ----");
    var ok = tryLoadFile("avg.avg");
    if (ok) {
        AVGPlayer.setDebugOutput(AVGPlayer.DEBUG_PROFILE | AVGPlayer.DEBUG_EVENTS2); 
        var node = AVGPlayer.getElementByID("nestedimg1");
        print("    Node id: "+node.getID());
        AVGPlayer.setInterval(10,"moveit();");
        AVGPlayer.setTimeout(5000,"quitTimeout();");
        AVGPlayer.play(30);
    }
}

function setExcl(i)
{
    print("switch: "+i);
    var node = AVGPlayer.getElementByID("switch");
    node.activeChild = i;
}

function testExcl()
{
    print ("---- testing excl node accessors ----");
    var ok = tryLoadFile("excl.avg");
    if (ok) {
        AVGPlayer.setTimeout(300,"setExcl(0);");
        AVGPlayer.setTimeout(600,"setExcl(1);");
        AVGPlayer.setTimeout(900,"setExcl(2);");
        AVGPlayer.setTimeout(1200,"setExcl(3);");
        AVGPlayer.setTimeout(1500,"quitTimeout()");
        AVGPlayer.play(30);
    }
}

//while (true) {
/*
    testAnimation();
    testWords();

    testCrop();
    testVideo();

    dumpNodes();
    testExcl();

    playAVG("image.avg");
    playAVG("empty.avg");
*/
    
    playAVG("events.avg");
/*
    playAVG("avg.avg");
    playAVG("noavg.avg");
    playAVG("noxml.avg");
*/
//}

