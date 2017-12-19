#pragma once

#include "open62541.h"

#define SOL_CHECK_ARGUMENTS 1
#include "sol/sol.hpp"

namespace lua_opcua {

void reg_opcua_enums(sol::table& module) {
	module.new_enum("UA_ClientState",
		"UA_CLIENTSTATE_DISCONNECTED", UA_ClientState::UA_CLIENTSTATE_CONNECTED,
		"UA_CLIENTSTATE_CONNECTED", UA_ClientState::UA_CLIENTSTATE_CONNECTED,
		"UA_CLIENTSTATE_SECURECHANNEL", UA_ClientState::UA_CLIENTSTATE_SECURECHANNEL,
		"UA_CLIENTSTATE_SESSION", UA_ClientState::UA_CLIENTSTATE_SESSION,
		"UA_CLIENTSTATE_SESSION_DISCONNECTED", UA_ClientState::UA_CLIENTSTATE_SESSION_DISCONNECTED
	);

}

}
