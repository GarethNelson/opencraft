//-----------------------------------------------------------------------------
//
// Copyright (C) 2017 by Gareth Nelson (gareth@garethnelson.com)
//
// This file is part of the OpenCraft client.
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
//     BaseCLI class
//
//-----------------------------------------------------------------------------

#pragma once

#include <opencraft/appfw/appfw.h>

#include <opencraft/appfw/interfaces/base_interface.h>

#include <opencraft/appfw/console/console_event_listener.h>

#include <string>

namespace opencraft { namespace appfw { namespace interfaces { namespace cli {

   class BaseCLI : public BaseInterface,opencraft::appfw::console::ConsoleEventListener {
       public:
          BaseCLI(opencraft::appfw::App *_app) : BaseInterface(_app) {};
          void Update();
          void GrabEvents();
          void on_output(std::string s);
          void on_output_clear() {};
          void on_input(std::string s) {};
          void on_input_clear() {};
       protected:
          bool first_update = true;

   };

}}}};
