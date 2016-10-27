#include <fstream>
#include <iostream>
#include <cstdlib>
#include "tcp.h"    // connect class

int main(int argc , char *argv[])
{
    tcp_client c;
    string data;
    int port = 1337;

    if(argc < 2) {
        cout << "Usage : "<< argv[0] << " <scriptfile> [port, default 1337]\n";
        return 1;
    }
    if(argc == 3){
        port = strtol(argv[2],NULL,10); 
    }

    if(!c.conn(HOST , port)){
        return 1;
    }

    string script = "";
    ifstream stream(argv[1]);
    string line;
    while(getline(stream, line)) {
        script=script+line;
    }
    stream.close();
    c.send_data(script.c_str());
    data = c.receive();
    cout <<data;
    cout << "\n";
       
    return 0;
}
