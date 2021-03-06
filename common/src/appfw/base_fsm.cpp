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
//     BaseFSM class
//
//-----------------------------------------------------------------------------

#include <opencraft/appfw/appstate/fsm/base_fsm.h>
#include <opencraft/appfw/appfw.h>

#include <opencraft/appfw/appstate/appvars/base_appvars.h>
#include <opencraft/appfw/console/logging/base_logger.h>
namespace opencraft { namespace appfw { namespace appstate { namespace fsm {

BaseFSM::BaseFSM(opencraft::appfw::App* _app) {
     this->app = _app;
     this->GlobalVars = new opencraft::appfw::appstate::appvars::BaseAppVars();
     this->LocalVars  = NULL;
     this->cur_state = NULL;
}

void BaseFSM::Update() {
     if(this->cur_state != NULL) {
       this->cur_state->Update();
     } else {
       this->Switch("IdleState");
     }
}

void BaseFSM::Switch(std::string state_name) {
     if(this->states.find(state_name) != this->states.end()) {
        if(this->cur_state == NULL) {
          OC_LOG_INFO(this->app,std::string("Starting with state ") + state_name);
          this->LocalVars = new opencraft::appfw::appstate::appvars::BaseAppVars();
        } else {
          OC_LOG_INFO(this->app,std::string("Switching state from ") + this->cur_state->GetName() + std::string(" to ") + state_name);
          delete this->LocalVars;
          this->LocalVars = NULL;
//        this->cur_state->Shutdown(); // TODO: implement this
        }
        this->cur_state = this->states[state_name];
        this->LocalVars = new opencraft::appfw::appstate::appvars::BaseAppVars();
     } else {
        OC_LOG_INFO(this->app,std::string("Could not switch to unknown state: ") + state_name);
     }
}

std::string BaseFSM::GetStateName() {
     if(this->cur_state != NULL) {
        return this->cur_state->GetName();
     } else {
        return "NULL";
     }
}

void BaseFSM::AddState(BaseState *state) {
     this->states[state->GetName()] = state;
     state->Init();
}



}}}};
