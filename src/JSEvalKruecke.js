function JSEvalKruecke(){}

JSEvalKruecke.prototype = {

    callEval: function (code) 
    { 
        return opener.eval(code);
    },

    QueryInterface: function (iid) {
        if (!iid.equals(Components.interfaces.IJSEvalKruecke)
            && !iid.equals(Components.interfaces.nsISupports))
        {
            throw Components.results.NS_ERROR_NO_INTERFACE;
        }
        return this;
    }
}


var Module = {
    firstTime: true,

    registerSelf: function (compMgr, fileSpec, location, type) {
        if (this.firstTime) {
            this.firstTime = false;
            throw Components.results.NS_ERROR_FACTORY_REGISTER_AGAIN;
        }
        compMgr =
            compMgr.QueryInterface(Components.interfaces.nsIComponentRegistrar);
        compMgr.registerFactoryLocation(this.myCID,
                                        "JSEvalKruecke",
                                        this.myProgID,
                                        fileSpec,
                                        location,
                                        type);
    },

    getClassObject : function (compMgr, cid, iid) {
        if (!cid.equals(this.myCID))
            throw Components.results.NS_ERROR_NO_INTERFACE
        if (!iid.equals(Components.interfaces.nsIFactory))
            throw Components.results.NS_ERROR_NOT_IMPLEMENTED;
        return this.myFactory;
    },

    myCID: Components.ID("{4ec47a4f-ef4a-4609-907b-7656d4208957}"),
    myProgID: "@c-base.org/jsevalkruecke;1",

    myFactory: {
        createInstance: function (outer, iid) {
            if (outer != null)
                throw Components.results.NS_ERROR_NO_AGGREGATION;
            return (new JSEvalKruecke()).QueryInterface(iid);
        }
    },

    canUnload: function(compMgr) {
        return true;
    }
}; 

function NSGetModule(compMgr, fileSpec) { return Module; }


