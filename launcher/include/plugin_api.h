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
//     Used to compile (and load) plugins
//
//-----------------------------------------------------------------------------

#pragma once

#include <version.h>
#include <common.h>  // although dynamically loaded, plugins might need the logger etc

#include <string>

// =============================================================================================================
//   PLUGIN-USABLE API HERE
// =============================================================================================================

// these functions are usable by loaded plugins and are implemented in the main launcher in plugin_api.cpp
// plugins should rely on these functions rather than reimplementing basic stuff like HTTP and JSON

// TODO: move this stuff into a seperate library

#include <cpr/cpr.h>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/value.h>

std::string read_url(std::string url);       // read the contents of a URL into memory, returns NULL on error
Json::Value json_from_string(std::string s); // parse a string into a JSON value

// =============================================================================================================
//   MODULE INFO STUFF HERE
// =============================================================================================================

typedef enum {MODTYPE_CLIENT,MODTYPE_SERVER} module_type_t;

// every module MUST include a module_info symbol of this struct type
typedef struct {
   module_type_t module_type;
   const char*   module_name;
   const char*   module_author;
   const char*   module_copyright;      // copyright and other legalese for the module itself, NOT supported clients/servers
   const char*   module_license;        // license for the module itself
   const char*   module_version;        // module version - later versions override earlier versions
   void          (*init_module)();      // this may be NULL if no init code is needed, otherwise it should be used to setup the client_api and other structs
   bool          (*check_for_update)(); // should query remote server and return true if an update is available, if not supported this should be set to NULL
} module_info_t;

// =============================================================================================================
//   CLIENT STUFF HERE
// =============================================================================================================

// used for client version information - ALL fields must be set unless stated otherwise, if not applicable use a 0-length string("")
// note that the client_name field should be identical for all versions of the same client software
// binary_repo_url can be anything and is used only by the plugin itself to derive a list of URLs to download
// the same plugin can support multiple pieces of software, but in practice different plugins should be used
typedef struct {
   bool        is_free;            // if the client version is free software or not
   bool        is_compatible;      // is this version compatible with this module?
   bool        is_installed;       // has this version been configured in a profile?
   const char* client_name;        // if the client has a specific name (e.g "Mojang official" or whatever), stick it here
   const char* client_license;     // if the client is free software, what license is it under? If not, ("")
   const char* version_id;         // a unique version string
   const char* binary_repo_url;    // used by the plugin internally, actual format of the content of this URL depends on client+plugin
   const char* source_repo_url;    // a git URL or NULL if nonfree
   const char* source_repo_branch; // a git branch/tag for this exact version or NULL if nonfree
   const char* protocol_versions;  // a comma-seperated list of supported protocol versions
} client_version_info_t;

// client plugins specify their API via this struct, which must be in the client_api symbol
// if a particular API function is not implemented it should be set to NULL
typedef struct {
   void                    (*update_version_data)();    // query remote server(s) and update internal list of available versions
   client_version_info_t** (*get_available_versions)(); // get known list of available versions as a NULL-terminated array
   client_version_info_t** (*get_supported_versions)(); // get known list of available+supported versions as a NULL-terminated array
   void                    (*download_version)(client_version_info_t *ver, const char* cache_path); // download specified client version into cache_path
} client_api_t;

// =============================================================================================================
//   SERVER STUFF HERE
// =============================================================================================================

// used for server version information
typedef struct {
   bool        is_free;           // if the server version is free software or not
   bool        is_compatible;     // is this version compatible with this module?
   bool        is_installed;      // has this version been configured in a profile?
   const char* client_license;    // if the server is free software, what license is it under?
   const char* version_id;        // a unique version string
   const char* source_repo_url;   // a git URL or NULL if no source is available
   const char* protocol_versions; // a comma-seperated list of supported protocol versions
} server_version_info_t;

// server plugins specify their API via this struct, which should be used for the server_api symbol
// as with client_api, if a particular API function is not implemented it should be set to NULL
typedef struct {
   void (*update_version_data)(); // query remote server(s) and update internal list of available versions
} server_api_t;
