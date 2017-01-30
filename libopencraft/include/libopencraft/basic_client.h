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
//     A basic client
//
//-----------------------------------------------------------------------------

#pragma once

#include <libopencraft/common.h>
#include <libopencraft/base_packet.h>
#include <libopencraft/packet_stream.h>

#include <string>
#include <map>
#include <vector>

namespace opencraft {
  namespace client {

typedef void (*pack_callback_t)(opencraft::packets::opencraft_packet *pack); // callbacks should NEVER delete/free the pack param lest bad things happen

class basic_client {
   public:
      basic_client();

      void register_handler(int32_t pack_ident, pack_callback_t cb);

      void on_recv(std::vector<unsigned char> data); // call this when data is available from the socket
      std::vector<unsigned char> on_send();          // call this to get data that should be transmitted to the server

   private:
      std::map<int32_t,std::vector<pack_callback_t> > pack_callbacks;
      int proto_mode;
      opencraft::packets::packet_stream p_stream;
};

}
}
