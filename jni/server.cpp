#include "server.h"

NetServer::NetServer(std::string filename)
    : fullname(filename)
    , fd( filename )
    , connect_status(false)
{
    base.netServerGotConnect_callback = netServerGotConnect_callback(nullptr);
    base.netServerLossConenct_callback = netServerLossConenct_callback(nullptr);
}

NetServer::~NetServer(){
    if( fd.is_open() ) {
        addEndCode();
        fd.close();
    }
}

bool NetServer::cache_one_packet(const Packet &packet){
    if(  fd.is_open()){
        fd.write( (char *)(packet.data), packet.size);
        return true;
    }
    return false;
}

bool NetServer::addEndCode(){
    if( fd.is_open() ){
        fd.write( endcode, sizeof(endcode) );
        return true;
    }        
    return false;
}