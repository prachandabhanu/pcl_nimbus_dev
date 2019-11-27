#include "websocket.h"

namespace nimbus{
    WebSocketClient::WebSocketClient(unsigned char * addr, bool continuousTrig = false, 
                    double port = 8080, double jsonPort = 8383, 
                    int rcvTimeout = 3, int pingTimeout = 3, 
                    int reconnectIntents = 3, double imgBufSize =10)
    {
        this->_address = addr;
        this->_streamPort = port;
        // this->_streamURL = "ws://%s:%d/stream", addr, port;
        this->_jsonPort = jsonPort;

        this->_rcvTimeout = rcvTimeout;
        this->_pingTimeout = pingTimeout;
        this->_reconnectIntents = reconnectIntents;

        this->_imgBufSize = imgBufSize;
        this->_listenStarted = false;
        this->_listenEnded = false;
        this->_connected = false;
        // thread
        this->_disconnectMe = false;

        // asyncio
        // image Queue

        double c_ = 299792458;
        double fmod = 11.78e6;
        this->_UR = c_/(2 * fmod);

        this->curl = curl_easy_init();

        //acquired thread
        //this->connect();
        // ..
    }

    WebSocketClient::~WebSocketClient(){}

    size_t WebSocketClient::WriteCallback(char *contents, size_t size, size_t nmemb, void *userp)
    {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    std::string WebSocketClient::_getJsonParameter(const char * data)
    {
        //CURL *curl = curl_easy_init();
        struct curl_slist *headers = NULL;
        CURLcode res;
        std::string readBuffer;
        unsigned char * url = this->_address;
        std::cout << url << std::endl;

        if(this->curl)
        {
            const char *payload = data;                           
            //"{\"jsonrpc\": \"2.0\", \"id\":\"0\", \"method\": \"getParameter\", \"params\": {\"component\": \"%s\", \"ID\": %d, \"param\": []} }", componet, paramID;
            headers = curl_slist_append(headers, "content-type: application/json;");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long) strlen(payload));
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &this->WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            res = curl_easy_perform(curl);

            if(res != CURLE_OK){
                fprintf(stderr, "curl_easy_persform() failed: %s\n", curl_easy_strerror(res));
            }
            curl_easy_cleanup(curl);
        }
        curl_global_cleanup();

        std::string input(readBuffer);
        std::vector<std::string> _reslut;
        boost::split(_reslut, input, boost::is_any_of(","));
        boost::split(_reslut, _reslut[3], boost::is_any_of(":"));
            //boost::split(_reslut, _reslut[1], boost::is_any_of(""));
        //std::cout << _reslut[1] << std::endl;
        //T final_r = _reslut[1];
        return _reslut[1];
    }


}