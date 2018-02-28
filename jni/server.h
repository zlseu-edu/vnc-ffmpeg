#ifndef SERVER_H
#define SERVER_H

#include <fstream>
#include <string>
#include "packetPool.h"

// mpeg file sequence end code.
static char endcode[] = { 0, 0, 1, 0xb7 };

typedef void (* netServerGotConnect_callback)(void);
typedef void (* netServerLossConenct_callback)(void);

typedef struct callback{
    void (* netServerGotConnect_callback)(void);
    void (* netServerLossConenct_callback)(void);
}net_callback_t;

class NetServer
{
public:
    NetServer(std::string filename);
    ~NetServer();
    void got_connect(){ 
        connect_status = true; 
        if(base.netServerGotConnect_callback)
            base.netServerGotConnect_callback();    
    }
    void loss_connect(){ 
        connect_status = false; 
        if(base.netServerLossConenct_callback)
            base.netServerLossConenct_callback();
    }

    void setGotConnect_callback(netServerGotConnect_callback func){
        base.netServerGotConnect_callback = func;
    }

    void setLossConnect_callback(netServerLossConenct_callback func){
        base.netServerLossConenct_callback = func;
    }

    bool addEndCode();
    bool cache_one_packet(const Packet &packet);

    bool waiting() { return !connect_status; }

private:
    net_callback_t base;
    std::string fullname;
    std::fstream fd;
    bool connect_status;
};
#endif