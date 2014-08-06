

var jsbridge = {
    assert: null,
    dataTypes: null,
    runTests: null,
};


(function(){
    
    jsbridge.assert = function(reason){
        native function assert();
        assert(reason);
    }

    jsbridge.dataTypes = function(strType, value){
        native function dataTypes();
        dataTypes(strType, value, "out");
    }

    //! v1: validate window.jsobject
    //! v2: validate window.cppobject
    //! dt: data types callback
    jsbridge.runTests = function(v1, v2, v3, dt){
        native function regCallbacks();
        regCallbacks(v1, v2, v3, dt);
    }
})();
