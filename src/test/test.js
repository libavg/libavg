
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

var timerid;

function playAVG (fileName) 
{
    print ("---- Playing "+fileName+"----");
    var ok = AVGPlayer.loadFile("../tests/"+fileName, new JSEvalKruecke());
    if (ok) {
        timerid = AVGPlayer.setInterval(40, "interval();");
//        AVGPlayer.setInterval(40, "interval();");
        AVGPlayer.setTimeout(2000, "timeout();");
        AVGPlayer.setTimeout(500, "AVGPlayer.clearInterval(timerid);");
        AVGPlayer.setTimeout(600, "print (\"600\");");
        AVGPlayer.setTimeout(700, "print (\"700\");");
        AVGPlayer.play();
    } else {
        print ("js: AVGPlayer.loadFile returned false");
    }

}


playAVG("image.avg");
playAVG("avg.avg");
playAVG("empty.avg");
playAVG("excl.avg");
playAVG("noavg.avg");
playAVG("noxml.avg");
playAVG("video.avg");

