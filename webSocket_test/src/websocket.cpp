#include "websocket.h"

#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <sstream>

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
        this->_disconnectMe = false;

        double c_ = 299792458;
        double fmod = 11.78e6;
        this->_UR = c_/(2 * fmod);

        // ..
        this->spread = this->getSpreadFactorXYZ<float_t>();
        int16_t * unitX = this->getUnitVectorX<int16_t *>();
        int16_t * unitY = this->getUnitVectorY<int16_t *>();
        int16_t * unitZ = this->getUnitVectorY<int16_t *>();
        
        // ToDo: Remove the tidious for loop!!
        for(size_t i = 0; i < 286*352; i++) this->_uX[i] = unitX[i] / spread;
        for(size_t i = 0; i < 286*352; i++) this->_uY[i] = unitY[i] / spread;
        for(size_t i = 0; i < 286*352; i++) this->_uZ[i] = unitZ[i] / spread;

        this->connect();  
    }

    WebSocketClient::~WebSocketClient()
    {
        this->_listenThread.join();
    }

    void WebSocketClient::listenerThread()
    {
        while(!this->_imageQueue.empty())
            this->_imageQueue.pop();
        int intent = 0;
        while (intent < this->_reconnectIntents)
        {
            std::string status = metadata.get()->get_status();
            if (status == "Open")
            {
                if(this->_imageQueue.size() <= this->_imgBufSize){
                    this->_imageQueue.push(metadata.get()->_queue);
                }else{
                    this->_imageQueue.pop();
                }
            }else if(status == "Connecting")
            {
                std::cout << "Connecting to the Web Socket Please wait:...!" << std::endl;
            }else if(status == "Failed"){
                std::cout << "Failed to connect .. Trying to reconnect again" << std::endl;
                intent +=1;
                //ToDo without exiting this thread this->connect();
            }
            
        }
    }

    void WebSocketClient::connect()
    {
        int id = endpoint.connect("ws://192.168.0.69:8080/stream");
        metadata = endpoint.get_metadata(id);
        if (metadata) {
            std::cout << metadata.get() << std::endl;
        } else {
            std::cout << "> Unknown connection id " << id << std::endl;
        }

        try
        {
            this->_listenThread = std::thread(&WebSocketClient::listenerThread,  this);
        }
        catch(const std::runtime_error &e)
        {
            std::cout << e.what() << std::endl;
        }  

    }

    void WebSocketClient::normalize(std::vector<std::vector<int16_t>> unit, std::vector<std::vector<float_t>> * _u){
         for (size_t j = 0; j < unit[0].size() ; j++)
        {
            std::vector<float_t> tempVector;
            for(size_t i = 0; i < unit.size(); i++)
            {
                int16_t temp = unit[i][j];
                float_t temp2 = temp / this->spread;
                tempVector.push_back(temp2);
            }
            _u->push_back(tempVector);
        }
     }

    template<typename D>
    D WebSocketClient::getUnitVectorX(){
        D resultValue;
        Json::Value result_r = this->_getJsonParameter<Json::Value>("{\"jsonrpc\": \"2.0\", \"id\":\"0\", \"method\": \"getParameter\", \"params\": {\"component\": \"nimbusRaw\", \"ID\": 4, \"param\": []} }");
        std::string rv(result_r["success"].asString());
        if(rv == "0")
        {
            std::string data(result_r["result"].asString());
            std::string decode = base64_decode(data);
            //std::u16string wDecode = std::u16string(decode.begin(), decode.end());
            int16_t* my_vec_x = (int16_t*)decode.c_str();
            /*std::vector<std::vector<int16_t> > my_2d_vec(286, std::vector<int16_t>(352, 0));
            //std::cout << "Row size: " << my_2d_vec.size() << "Column size: " << my_2d_vec[0].size() << std::endl;
            int counter = 0;
            for (size_t j = 0; j < my_2d_vec[0].size(); j++)           //column
            {
                for(size_t i = 0; i <my_2d_vec.size() ; i++)        //Row
                {
                    my_2d_vec[i][j] = my_vec_x[counter];
                    counter ++;
                }
            }*/
            resultValue =  my_vec_x;
        }
        return (resultValue);
    }
    template<typename D>
    D WebSocketClient::getUnitVectorY(){
        D resultValue;
        Json::Value result_r = this->_getJsonParameter<Json::Value>("{\"jsonrpc\": \"2.0\", \"id\":\"0\", \"method\": \"getParameter\", \"params\": {\"component\": \"nimbusRaw\", \"ID\": 5, \"param\": []} }");
        std::string rv(result_r["success"].asString());
        if(rv == "0")
        {
            std::string data(result_r["result"].asString());
            std::string decode = base64_decode(data);
            //std::u16string wDecode = std::u16string(decode.begin(), decode.end());
            int16_t* my_vec_y = (int16_t*)decode.c_str();
            /*std::vector<std::vector<int16_t> > my_2d_vec(286, std::vector<int16_t>(352, 0));
            //std::cout << "Row size: " << my_2d_vec.size() << "Column size: " << my_2d_vec[0].size() << std::endl;
            int counter = 0;
            for (size_t j = 0; j < my_2d_vec[0].size(); j++)           //column
            {
                for(size_t i = 0; i <my_2d_vec.size() ; i++)        //Ros
                {
                    my_2d_vec[i][j] = my_vec_y[counter];
                    counter ++;
                }
            }*/
            resultValue =  my_vec_y;
        }
        return (resultValue);
    }
    template<typename D>
    D WebSocketClient::getUnitVectorZ(){
        D resultValue;
        Json::Value result_r = this->_getJsonParameter<Json::Value>("{\"jsonrpc\": \"2.0\", \"id\":\"0\", \"method\": \"getParameter\", \"params\": {\"component\": \"nimbusRaw\", \"ID\": 6, \"param\": []} }");
        std::string rv(result_r["success"].asString());
        if(rv == "0")
        {
            std::string data(result_r["result"].asString());
            std::string decode = base64_decode(data);
            //std::u16string wDecode = std::u16string(decode.begin(), decode.end());
            int16_t* my_vec_z = (int16_t*)decode.c_str();
            /*std::vector<std::vector<int16_t> > my_2d_vec(286, std::vector<int16_t>(352, 0));
            //std::cout << "Row size: " << my_2d_vec.size() << "Column size: " << my_2d_vec[0].size() << std::endl;
            int counter = 0;
            for (size_t j = 0; j < my_2d_vec[0].size(); j++)           //column
            {
                for(size_t i = 0; i <my_2d_vec.size() ; i++)        //Ros
                {
                    my_2d_vec[i][j] = my_vec_z[counter];
                    counter ++;
                }
            }*/
            resultValue =  my_vec_z;
        }
        return (resultValue);
    }

    template<typename D>
    D WebSocketClient::getSpreadFactorXYZ()
    {
        D returnDouble;
        Json::Value result_r = this->_getJsonParameter<Json::Value>("{\"jsonrpc\": \"2.0\", \"id\":\"0\", \"method\": \"getParameter\", \"params\": {\"component\": \"nimbusRaw\", \"ID\": 7, \"param\": []} }");
        std::string rv(result_r["success"].asString());
        if(rv == "0"){
            std::string data(result_r["result"].asString());
            double dResult = std::stod(data);
            returnDouble = dResult;
        }
        return (returnDouble);
    }

    size_t WebSocketClient::WriteCallback(const char* in, std::size_t size, std::size_t num, std::string* out)
    {
        const std::size_t totalBytes(size * num);
        out->append(in, totalBytes);
        return totalBytes;
    }

    template<typename T, typename D>
    T WebSocketClient::_getJsonParameter(D data)
    {
        CURL *curl = curl_easy_init();
        struct curl_slist *headers = NULL;
        CURLcode res;
        std::string readBuffer;
        unsigned char * url = this->_address;       //ToDo Change this to string
        long httpCode(0);
        std::unique_ptr<std::string> httpData(new std::string());
        const std::string result_string;
        T resultJson;

        if(curl)
        {
            /** ToDo:
             * @brief Use the json paeser construct that payload and use this as string!!
             */
            const char *payload = data; 
            headers = curl_slist_append(headers, "content-type: application/json;");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long) strlen(payload));
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &this->WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, httpData.get());

            res = curl_easy_perform(curl);
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
            if(res != CURLE_OK){
                fprintf(stderr, "curl_easy_persform() failed: %s\n", curl_easy_strerror(res));
            }
            curl_easy_cleanup(curl);
        }

        if(httpCode == 200)
        {
            Json::Value jsonData;
            Json::Reader jsonReader;
            if(jsonReader.parse(*httpData.get(), jsonData))
            {
                jsonReader.parse(jsonData["result"].toStyledString(), jsonData);
                const std::string dataString(jsonData["result"].asString());
                resultJson = jsonData;
            }else
            {
                std::cout << "Could not parse HTTP data as JSON" << std::endl;
                std::cout << "HTTP data was:\n" << *httpData.get() << std::endl;
                return "error";
            }
            
        }else
        {
            std::cout << "Couldn't GET from " << url << " - exiting" << std::endl;
            return "error";
        }
        curl_global_cleanup();
        return (resultJson);
    }
}