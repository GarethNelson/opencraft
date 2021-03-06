//-----------------------------------------------------------------------------
//
// Copyright (C) 2017 by Gareth Nelson (gareth@garethnelson.com)
//
// This file is part of OpenCraft.
//
// The OpenCraft client is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// The OpenCraft client is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with OpenCraft.  If not, see <http://www.gnu.org/licenses/>.
//
// DESCRIPTION:
//     Connects to server using name specified on command line and allows chat
//
//-----------------------------------------------------------------------------


#include <libopencraft/packets.autogen.h>
#include <libopencraft/version.h>
#include <libopencraft/packet_reader.h>
#include <libopencraft/packet_writer.h>
#include <libopencraft/proto_constants.h>

#include <boost/thread.hpp>

#include <jsoncpp/json/value.h>
#include <jsoncpp/json/reader.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>

#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <exception>
#include <vector>

using namespace std;
using namespace opencraft::packets;

std::string chat_input;

bool running;
bool ingame = false;
int sockfd;



void chat_in() {
     opencraft::packets::packet_writer chat_writer(sockfd);
     while(!ingame) usleep(1000);
     while(running) {
        cout << "Chat> ";
        getline(cin,chat_input);
        if(chat_input.size()>0) {
             
          if(chat_input.compare("/quit")==0) { 
             running=false;
          } else {
             chat_message_play_upstream chat_msg(chat_input);
             chat_writer.write_pack(&chat_msg);
          }
          chat_input.erase();
       }
     }
}

void player_tick() {
     opencraft::packets::packet_writer tick_writer(sockfd);
     while(!ingame) usleep(1000);
     while(running) {
       usleep(50000);
       player_play_upstream play(true);
       tick_writer.write_pack(&play);
    }

}

int main(int argc, char** argv) {
    // setup socket
    running = true;
    sockfd = socket(AF_INET, SOCK_STREAM,0);
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    server_addr.sin_port = htons(25565);

    // connect to localhost:25565 and then setup a packet reader and writer
    connect(sockfd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_in));
    opencraft::packets::packet_reader client_reader(sockfd,OPENCRAFT_STATE_LOGIN,true);

    opencraft::packets::packet_writer hs_writer(sockfd);

    // create a handshake packet and write it
    handshake_handshaking_upstream hspack(OPENCRAFT_PROTOCOL_VERSION,std::string(OPENCRAFT_DEFAULT_SERVER),OPENCRAFT_DEFAULT_TCP_PORT,OPENCRAFT_STATE_LOGIN);
    hs_writer.write_pack(&hspack);
     
    cout << "Sent handshake" << endl;

    // create a login packet and write it
    std::string name = argv[1];


    login_start_login_upstream login_req(name);
    opencraft::packets::packet_writer login_writer(sockfd);
    login_writer.write_pack(&login_req);

    cout << "Sent login" << endl;


    bool waiting = true;
    // read the login success packet and dump it
    opencraft_packet *_inpack = NULL;
    login_success_login_downstream *login_succ = NULL;

    while(waiting) {
      _inpack = NULL;
      _inpack = client_reader.read_pack();
      cout << "ident " << _inpack->ident() << endl;
      cout << "Got " << _inpack->name() << endl;
      cout << _inpack->dump_hex() << endl;
      int32_t pack_ident = _inpack->ident();
      switch(pack_ident) {
         case OPENCRAFT_PACKIDENT_LOGIN_DISCONNECT_LOGIN_DOWNSTREAM: {
           login_disconnect_login_downstream* p=(login_disconnect_login_downstream*)_inpack;
           cout << p->a << endl;
           abort();
         break;}
         case OPENCRAFT_PACKIDENT_LOGIN_SUCCESS_LOGIN_DOWNSTREAM: {
           login_succ = (login_success_login_downstream*)_inpack;
           waiting = false;
         break;}
      }
    }

    
    cout << "Got UUID " << login_succ->a << endl;

    // switch to play mode
    client_reader.proto_mode = OPENCRAFT_STATE_PLAY;

    // read packets and spit out chat messages
    ingame = false;
    boost::thread chat_t{chat_in};
    boost::thread tick_t{player_tick};

    cout << "Waiting for avatar spawn..." << endl;
    while(running) {
       opencraft_packet* inpack = NULL;
       inpack = client_reader.read_pack();
       if(inpack != NULL) {
          if(inpack->name().compare("unknown")!=0) {
             int32_t pack_ident = inpack->ident();
             switch(pack_ident) {
                case OPENCRAFT_PACKIDENT_PLAYER_POSITION_AND_LOOK_PLAY_DOWNSTREAM: {
                     int32_t teleport_id = ((player_position_and_look_play_downstream*)inpack)->g;
                     double  x           = ((player_position_and_look_play_downstream*)inpack)->a;
                     double  y           = ((player_position_and_look_play_downstream*)inpack)->b;
                     double  z           = ((player_position_and_look_play_downstream*)inpack)->c;
                     teleport_confirm_play_upstream teleport_pack(teleport_id);
//                     client_writer.write_pack(&teleport_pack);
                     ingame = true;
                     break;
                }
                case OPENCRAFT_PACKIDENT_CHAT_MESSAGE_PLAY_DOWNSTREAM: {
                     std::string msg = ((chat_message_play_downstream*)inpack)->a;
                     Json::Value chatmsg;
                     std::stringstream json_stream;
                     json_stream << msg;
                     json_stream >> chatmsg;
                     cout << chatmsg["extra"][0]["text"].asString() << endl;
                     break;
                }
                case OPENCRAFT_PACKIDENT_KEEP_ALIVE_PLAY_DOWNSTREAM: {
                     int32_t ack_id = ((keep_alive_play_downstream*)inpack)->a;
                     keep_alive_play_upstream ack_pack(ack_id);
//                     client_writer.write_pack(&ack_pack);
                     break;
                }
             }
          }
          delete inpack;
       }
       
    }
}




