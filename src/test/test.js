
var PlayerFactory = Components.Constructor("@c-base.org/avgplayer;1","IAVGPlayer");
var AVGPlayer = new PlayerFactory();

function JSEvalKruecke(){} // constructor
JSEvalKruecke.prototype = {

    callEval: function (code) 
    { 
        return eval(code);
    },

    QueryInterface: function (iid) {
        if (!iid.equals(Components.interfaces.IJSEvalKruecke))
        {
            throw Components.results.NS_ERROR_NO_INTERFACE;
        }
        return this;
    }
}

function timeout()
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
    print ("JS: onMouseMove()");
}
function onMouseOver()
{
    print ("JS: onMouseOver()");
}
function onMouseOut()
{
    print ("JS: onMouseOut()");
}

function onMouseUp()
{
    var Event = AVGPlayer.getCurEvent();
    print ("JS: onMouseUp Event (type= "+Event.getType()+", pos=("+Event.getXPos()+
           ", "+Event.getYPos()+"), state="+Event.getMouseButtonState()+")");
}

var timerid;

function playAVG (fileName) 
{
    print ("---- Playing "+fileName+"----");
    var ok = AVGPlayer.loadFile("../tests/"+fileName, new JSEvalKruecke());
    AVGPlayer.setEventDebugLevel(2);
    if (ok) {
        timerid = AVGPlayer.setInterval(40, "interval();");
        AVGPlayer.setTimeout(5000, "timeout();");
        AVGPlayer.setTimeout(500, "AVGPlayer.clearInterval(timerid);");
        AVGPlayer.setTimeout(1000, "AVGPlayer.setEventDebugLevel(1);");
        AVGPlayer.setTimeout(2000, "AVGPlayer.setEventDebugLevel(0);");
        AVGPlayer.play();
    } else {
        print ("js: AVGPlayer.loadFile returned false");
    }

}


playAVG("events.avg");
playAVG("image.avg");
playAVG("avg.avg");
playAVG("empty.avg");
playAVG("excl.avg");
playAVG("noavg.avg");
playAVG("noxml.avg");
playAVG("video.avg");

