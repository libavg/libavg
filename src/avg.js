
var PlayerFactory = Components.Constructor("@c-base.org/avgplayer;1","IAVGPlayer");
var AVGPlayer = new PlayerFactory();

function JSEvalKruecke(){} // constructor
JSEvalKruecke.prototype = {

    callEval: function (code) 
    { 
        try {
            return eval(code);
        } catch (e) {
            print (e.name + ": " + e.message + " while evaluating " + code);
            
        }
    },

    QueryInterface: function (iid) {
        if (!iid.equals(Components.interfaces.IJSEvalKruecke))
        {
            throw Components.results.NS_ERROR_NO_INTERFACE;
        }
        return this;
    }
}

