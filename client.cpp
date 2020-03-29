#include <arpa/inet.h>
#include "helpers.h"
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h> 
#include <iostream>
#include <cstring>
#include <vector>
#include <algorithm>
#include "json.hpp"
#include <sstream>

using json = nlohmann::json;
using namespace std;

static string& implode(const vector<string>& elems, char delim, string& s)
{
    for (vector<string>::const_iterator ii = elems.begin(); ii != elems.end(); ++ii)
    {
        s += (*ii);
        if ( ii + 1 != elems.end() ) {
            s += delim;
        }
    }

    return s;
}

static string implode(const vector<string>& elems, char delim)
{
    string s;
    return implode(elems, delim, s);
}

struct request 
{
    string type;
    vector <string> headers;
    string content;
    string path;
    string host;
    string port;
};

string get_request_string(struct request req)
{
    string my_json;
    string host;
    string headers;
    string req_final;

    if((req.type).compare("GET") == 0)
        my_json = req.type + " " + req.path + "?"+ req.content + " " + "HTTP/1.1\n";
    else
        my_json = req.type + " " + req.path + " " + "HTTP/1.1\n";
    
    host =  "Host: " + req.host + ":" + req.port + "\n";
    headers = implode(req.headers, '\n');

    if(!headers.empty())
        headers += "\n";

    if((req.type).compare("GET") == 0)
        req_final = my_json + host + headers + "\n";
    else
        req_final = my_json + host + headers + "Content-Length: " + to_string((req.content).length()) + "\n\n" + req.content;

    return req_final;
}



struct request add_header(struct request req, string header)
{
    (req.headers).push_back(header);
    return req;
}

char* do_request(struct request req)
{
    char *message;
    char *response;
    int sockfd;

    sockfd = open_connection((char*)req.host.c_str(), stoi(req.port), AF_INET, SOCK_STREAM, 0);
    message = strdup(get_request_string(req).c_str());
    send_to_server(sockfd,message);
    response = receive_from_server(sockfd);
    close_connection(sockfd);
    free(message);
    return response;
}

string return_body(string response)
{
    return response.substr(response.find("\r\n\r\n") + 4, response.length());
}

string return_header(string response)
{
    return response.substr(0, response.find("\r\n\r\n"));
}

string strip_quotes(string str)
{
    str.erase(remove(str.begin(), str.end(), '"'), str.end());
    return str;
}

struct request add_cookies(struct request req, string header)
{
    stringstream iss(header);
    string SingleLine;

    while(iss.good())
    {
        getline(iss,SingleLine,'\n');
        if (SingleLine.find("Set-Cookie:") != string::npos)
        {
            SingleLine.erase(0,4);
            req = add_header(req, SingleLine);
        } 
    }

    return req;
}

char* dns(char * hostname)
{
    struct hostent *he;
    struct in_addr **addr_list;
    
    he = gethostbyname( hostname );
    addr_list = (struct in_addr **) he->h_addr_list;
    
    return inet_ntoa(*addr_list[0]);
}

int main(int argc, char *argv[])
{
    
    /* requestul initial */
    struct request req;
    req.type = "GET";
    req.host = "185.118.200.35";
    req.port = "8081";
    req.path = "/task1/start";

    string task1(do_request(req));
    cout<<endl<<"============================task1============================"<<endl;
    cout<<task1<<endl;
    // cout<<task1;
    json my_json = json::parse(return_body(task1)); 
  
    /* task 2 */
    req.path = strip_quotes(my_json["url"]);
    req.type=strip_quotes(my_json["method"]);
    req.content = "username=" + strip_quotes((my_json["data"])["username"]) + "&password=" + strip_quotes((my_json["data"])["password"]);
    req = add_header(req, "Content-Type: " + strip_quotes(my_json["type"]));
    req = add_cookies(req, return_header(task1));

    cout<<endl<<"============================task2============================"<<endl;
    cout<<get_request_string(req)<<endl;
    string task2(do_request(req));
    req.headers.clear();
    cout<<endl<<"============================Raspuns============================"<<endl;
    cout<<task2<<endl;

    /* task3 */
    my_json = json::parse(return_body(task2));
    req.path = strip_quotes(my_json["url"]);
    req.type=strip_quotes(my_json["method"]);
    req.content = "raspuns1=omul&raspuns2=numele&id=" + strip_quotes(((my_json["data"])["queryParams"])["id"]);
    string token = "Authorization: token " + strip_quotes((my_json["data"])["token"]);
    req = add_header(req, token);
    req = add_cookies(req, return_header(task2));
    cout<<endl<<"============================task3============================"<<endl;
    cout<<get_request_string(req)<<endl;
    string task3(do_request(req));
    cout<<endl<<"============================Raspuns============================"<<endl;
    cout<<task3<<endl;

    /* task4 */
    my_json = json::parse(return_body(task3));
    req.path = strip_quotes(my_json["url"]);
    req.type=strip_quotes(my_json["method"]);
    req.content.clear();
    req.headers.clear();
    req = add_header(req, token);
    req = add_cookies(req, return_header(task3));
    cout<<endl<<"============================task4============================"<<endl;
    cout<<get_request_string(req)<<endl;
    string task4(do_request(req));
    cout<<endl<<"============================Raspuns============================"<<endl;
    cout<<endl<<task4<<endl;

    /* task5 */
    my_json = json::parse(return_body(task4));
    cout<<endl<<"============================task5============================"<<endl;
    struct request req_weather;
    req_weather.type=strip_quotes((my_json["data"])["method"]);
    req_weather.path = strip_quotes((my_json["data"])["url"]).substr(strip_quotes((my_json["data"])["url"]).find("/"));
    req_weather.host = dns((char*)strip_quotes((my_json["data"])["url"]).substr(0, strip_quotes((my_json["data"])["url"]).find("/")).c_str());
    req_weather.headers.clear();
    req_weather.content = "q=" + strip_quotes(((my_json["data"])["queryParams"])["q"])
                 + "&APPID=" + strip_quotes(((my_json["data"])["queryParams"])["APPID"]);
    req_weather.port = "80";
    cout<<get_request_string(req_weather)<<endl;
    string task5_weather(do_request(req_weather));
    cout<<endl<<"============================Raspuns============================"<<endl;
    cout<<endl<<task5_weather;
    string json_weather = return_body(task5_weather);

    req.content = json_weather;
    req.path = strip_quotes(my_json["url"]);
    req.type=strip_quotes(my_json["method"]);
    req.headers.clear();
    req = add_cookies(req, return_header(task4));
    req = add_header(req, "Content-Type: " + strip_quotes(my_json["type"]));
    req = add_header(req, token);

    cout<<endl<<get_request_string(req)<<endl;
    cout<<endl<<"============================Raspuns============================"<<endl;
    string task5(do_request(req));
    cout<<endl<<task5<<endl;
    return 0;
}
