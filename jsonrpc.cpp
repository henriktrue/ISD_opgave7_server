#include "ArduinoJson/ArduinoJson.h"
#include <iostream>
#include <syslog.h>
#include <string>
#include <cstring>
#include "sqlite.h"
using namespace std;

int jsonrpc_debug(char * jsonrpc) {
    DynamicJsonBuffer jsonBuffer;
    /*Allocates and populate a JsonObject called root from a JSON string, 
     * returning a reference to the newly created JsonObject
    */
    
    string jsonrpc_version = root["jsonrpc"];
    string method = root["method"];
    JsonVariant params = root["params"];
    long id = root["id"];

    /*Tells if the array is valid or not, which can be used to check if the 
     array is successfully parsed, or to check the array successfully allocated.
    */ 
   
    if (!root.success()) {
        cout << "Not a valid JSON object" << endl;
        return -1;
    }

    if (jsonrpc_version != "2.0" || !method.length()) {
        cout << "Not a valid JSONRPC object" << endl;
        return -1;
    }

    cout << "JSONRPC begins" << endl;
    cout << "Version: " << jsonrpc_version << endl;

    cout << "Method: " << method << endl;
    //tests if argument is JsonArray
    if (params.is<JsonArray>()) {
        cout << "Parameter Array" << endl;
        int i = 0;
        for (const auto& kv : params.as<JsonObject>()) {
            cout << "[" << kv.key << "] " << kv.value.as<char*>() << endl;
        }
    }
    if (params.is<JsonObject>()) {
        cout << "Parameter Object" << endl;
        for (const auto& kv : params.as<JsonObject>()) {
            cout << kv.key << " -> " << kv.value.as<char*>() << endl;
        }
    }

    cout << "END OF JSONRPC" << endl;
    return 0;
}

string jsonrpc_handler(string jsonrpc) {
    //For debugging purposes
    //cout << "Dump: ##" << jsonrpc << "##" << endl;
    StaticJsonBuffer<500> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(jsonrpc);

    string jsonrpc_version = root["jsonrpc"];
    string method = root["method"];
    JsonVariant params = root["params"];
    long id = root["id"];

    if (!root.success()) {
        syslog(LOG_INFO, "Not a valid JSON object, -32700\n");
        return "{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32700, \"message\": \"Parse error\"}, \"id\": null}\n";
    }

    if (jsonrpc_version != "2.0" || !method.length()) {
        syslog(LOG_INFO, "Not a valid JSONRPC object, -32600\n");
        return "{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32600, \"message\": \"Invalid request\"}, \"id\": null}\n";
    }

    if (method == "Read_Latest_Val") {
        root["result"] = sqlite_getlatest();
        
    } else if (method == "Store_Val") {
        sqlite_insert(params);
    }else{
         return "{\"jsonrpc\": \"2.0\","
                 " \"error\": {\"code\": -32601,"
                 " \"message\": \"The method does not exist/is not available.\"},"
                 " \"id\": null}\n";
    }

    root.remove("method");
    root.remove("params");
    string output;
    root.printTo<string>(output);
    output.append("\n");
    return output;
}