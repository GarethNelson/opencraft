//-----------------------------------------------------------------------------
//
// Copyright (C) 2017 by Gareth Nelson (gareth@garethnelson.com)
//
// This file is part of the OpenCraft server.
//
// The OpenCraft server is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// The OpenCraft server is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with OpenCraft.  If not, see <http://www.gnu.org/licenses/>.
//
// DESCRIPTION:
//     The daemon class
//
//-----------------------------------------------------------------------------

#include <logging.h>
#include <opencraft_daemon.h>

#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include <exception>
#include <chrono>
#include <iostream>
#include <ctime>
#include <cstdint>
#include <string>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <wordexp.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <event2/event.h>
#include <event2/thread.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include <string>

using namespace std;

opencraft_daemon::opencraft_daemon(bool debug_mode, bool daemon_mode, std::string pidfile, std::string install_root, int listen_port) {
   this->_debug_mode      = debug_mode;
   this->_daemon_mode     = daemon_mode;
   this->_pidfile         = pidfile;
   this->_install_root    = install_root;
   this->_listen_port     = listen_port;
   this->_active          = false;
}

double opencraft_daemon::mticks() {
    typedef std::chrono::high_resolution_clock clock;
    typedef std::chrono::duration<float, std::milli> duration;

    static clock::time_point start = clock::now();
    duration elapsed = clock::now() - start;

    return elapsed.count();
}

void accept_conn_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address, int socklen, void *ctx) {
     opencraft_daemon* d = (opencraft_daemon*)ctx;
     d->_io_service.post(boost::bind(&opencraft_daemon::accept_client_cb, d, listener, fd, address, socklen));
}

void read_ready_cb(evutil_socket_t fd, short what, void* arg) {
     client_connection* client = (client_connection*)arg;
     client->_daemon->_io_service.post(boost::bind(&client_connection::read_cb, client, fd));
}

void write_ready_cb(evutil_socket_t fd, short what, void* arg) {
     client_connection* client = (client_connection*)arg;
     client->_daemon->_io_service.post(boost::bind(&client_connection::write_cb, client, fd));
}

std::string opencraft_daemon::get_status_json() {
     Json::Value Version;
     Version["name"]     = "OpenCraft";
     Version["protocol"] = OPENCRAFT_PROTOCOL_VERSION;
     
     Json::Value Players;
     Players["online"]   = 0;   // TODO: fix this to actually count clients connected
     Players["max"]      = 100; // TODO: make this a configurable variable

     Json::Value Description;
     Description["text"] = "OpenCraft server";

     Json::Value resp_val;
     resp_val["version"]     = Version;
     resp_val["players"]     = Players;
     resp_val["description"] = Description;
     
     Json::FastWriter Writer;
     std::string resp_str = Writer.write(resp_val);
     return resp_str;
}


void opencraft_daemon::accept_client_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address, int socklen) {
     char addr_ip[200];
     evutil_inet_ntop(AF_INET, &(((sockaddr_in*)address)->sin_addr), addr_ip, 200); 
     uint16_t addr_port = ntohs( ((struct sockaddr_in*)address)->sin_port);

     std::string client_addr = std::string(addr_ip) + ":" + to_string(addr_port);

     client_connection *new_conn  = new client_connection(client_addr,this);
     this->_clients[client_addr] = new_conn;

     LOG(info) << "New connection from " << client_addr;

     new_conn->read_ev  = event_new(this->ev_base,fd,EV_READ|EV_PERSIST,read_ready_cb,new_conn);
     new_conn->write_ev = event_new(this->ev_base,fd,EV_WRITE|EV_PERSIST,write_ready_cb,new_conn);
     event_add(new_conn->read_ev,NULL);
     event_add(new_conn->write_ev,NULL);
}

void opencraft_daemon::run() {
     this->write_pidfile(this->_pidfile);
     if(this->_daemon_mode) {
        LOG(info) << "Daemonizing...";
        this->setup_daemon();
        this->configure_stdio();
        this->configure_signals();
     }
     
     LOG(info) << "Configuring threadpool...";
     boost::asio::io_service::work work(this->_io_service);
     for (std::size_t i = 0; i < 8; ++i) { // TODO: make this configurable again
         this->_thread_pool.create_thread(boost::bind(&boost::asio::io_service::run, &(this->_io_service)));
     }

     LOG(info) << "Configuring libevent...";
     LOG(info) << "Compiled with libevent " << LIBEVENT_VERSION;
     LOG(info) << "Running with libevent " << event_get_version();
     if(evthread_use_pthreads()==-1) {
        LOG(fatal) << "Could not configure pthreads for libevent!";
        abort();
     }

     this->ev_base = event_base_new();

     LOG(info) << "Configuring network port...";
     struct sockaddr_in sin;
     memset(&sin, 0, sizeof(sin));
     sin.sin_family = AF_INET;
     sin.sin_addr.s_addr = htonl(0);
     sin.sin_port = htons(this->_listen_port);

     this->ev_listener = evconnlistener_new_bind(this->ev_base, accept_conn_cb, (void*)this, LEV_OPT_THREADSAFE|LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1, (struct sockaddr*)&sin, sizeof(sin));

     LOG(info) << "Entering mainloop...";
     this->_active = true;

     // configure time interval for checking libevent events
     struct timeval tick_interval;
     tick_interval.tv_sec  = 0;
     tick_interval.tv_usec = 25000;
     event_base_loopexit(this->ev_base, &tick_interval);

     

     double last_tick  = mticks();
     double tick_delta = 0.0;
     while(this->_active) {
        // first dispatch networking events for half a tick
        event_base_dispatch(this->ev_base);

        // cleanup any inactive clients
        for(int i=0; i < this->_inactive_clients.size(); i++) {
               delete this->_clients[this->_inactive_clients[i]];
               this->_clients.erase(this->_inactive_clients[i]);
        }
        this->_inactive_clients.clear();

        // calculate if it's time to run another tick yet (if not, the loop simply repeats and we do more networking events)
        tick_delta = mticks() - last_tick;
        if(tick_delta >= 50) { // if 50ms or more have passed since the last tick, it's time to run another one
           last_tick = mticks();
           // DO PER-TICK LOGIC HERE
        }
       
     }
}

void opencraft_daemon::fork_me_baby() { // harder, yeah
     if(fork() >0) exit(0);
}

void opencraft_daemon::write_pidfile(std::string filename) {
     wordexp_t exp_result;
     wordexp(filename.c_str(), &exp_result, 0);
     FILE* fd = fopen(exp_result.we_wordv[0],"w");
     wordfree(&exp_result);
     fprintf(fd,"%d\n",getpid());
     fclose(fd);
}

void opencraft_daemon::setup_daemon() {
     this->fork_me_baby();
     chdir("/");
     setsid();
     umask(0);
     this->fork_me_baby(); // fork me harder, oh yeah
}

void opencraft_daemon::configure_stdio() {
     int i;
     for (i = getdtablesize(); i >= 0; --i) {
            close(i);
     }
     i = open("/dev/null", O_RDWR);
     dup(i);
     dup(i);
}

void opencraft_daemon::configure_signals() {
     sigset_t         signal_set;
     struct sigaction sig_action;

     sigemptyset(&signal_set);
     sigaddset(&signal_set, SIGCHLD);
     sigaddset(&signal_set, SIGTSTP);
     sigaddset(&signal_set, SIGTTOU);
     sigaddset(&signal_set, SIGTTIN);
     sigaddset(&signal_set, SIGPIPE);
     sigprocmask(SIG_BLOCK, &signal_set, NULL);

     // TODO: implement the below properly and add a signal handler with a static reference or something
     /*
     sig_action.sa_handler = signal_handler;
     sigemptyset(&sig_action.sa_mask);

     sig_action.sa_flags = 0;
     sigaction(SIGHUP,  &sig_action, NULL);
     sigaction(SIGTERM, &sig_action, NULL);
     sigaction(SIGINT,  &sig_action, NULL);
     */
}