#pragma once
#include <iomanip>
//#include <iostream>

#include "open62541.h"

#define SOL_CHECK_ARGUMENTS 1
#include "sol/sol.hpp"

#include "opcua_interfaces.hpp"

namespace lua_opcua {

typedef std::function<void (UA_LogLevel level, UA_LogCategory category, const char *msg)> LogStdFunction;

std::string toString(const UA_Guid& guid) {
	std::stringstream ss;
	ss << std::hex << std::setw(8) <<std::setfill('0')  << guid.data1;
	ss << std::hex << std::setw(4) <<std::setfill('0')  << guid.data2;
	ss << std::hex << std::setw(4) <<std::setfill('0')  << guid.data4;
	return ss.str();
}
std::string toString(const UA_NodeId& id) {
	std::stringstream ss;
	ss << "NodeId(ns=" << id.namespaceIndex << ";i=";
	switch(id.identifierType) {
		case UA_NODEIDTYPE_GUID:
			ss << toString(id.identifier.guid);
			break;
		case UA_NODEIDTYPE_STRING:
			ss << std::string((char*)id.identifier.string.data, id.identifier.string.length);
			break;
		case UA_NODEIDTYPE_BYTESTRING:
			ss << std::string((char*)id.identifier.byteString.data, id.identifier.byteString.length);
			break;
		case UA_NODEIDTYPE_NUMERIC:
		default:
			ss << id.identifier.numeric;
			break;
	}
	return ss.str();
}

void reg_opcua_types(sol::table& module) {
	module.new_enum("VariantType",
		"BOOLEAN", UA_TYPES_BOOLEAN,
		"SBYTE", UA_TYPES_SBYTE,
		"BYTE", UA_TYPES_BYTE,
		"INT16", UA_TYPES_INT16,
		"UINT16", UA_TYPES_UINT16,
		"INT32", UA_TYPES_INT32,
		"UINT32", UA_TYPES_UINT32,
		"INT64", UA_TYPES_INT64,
		"UINT64", UA_TYPES_UINT64,
		"FLOAT", UA_TYPES_FLOAT,
		"DOUBLE", UA_TYPES_DOUBLE,
		"STRING", UA_TYPES_STRING,
		"DATETIME", UA_TYPES_DATETIME,
		"GUID", UA_TYPES_GUID,
		"BYTESTRING", UA_TYPES_BYTESTRING,
		"XMLELEMENT", UA_TYPES_XMLELEMENT,
		"NODEID", UA_TYPES_NODEID,
		"EXPANDEDNODEID", UA_TYPES_EXPANDEDNODEID,
		"STATUSCODE", UA_TYPES_STATUSCODE,
		"QUALIFIEDNAME", UA_TYPES_QUALIFIEDNAME,
		"LOCALIZEDTEXT", UA_TYPES_LOCALIZEDTEXT,
		"EXTENSIONOBJECT", UA_TYPES_EXTENSIONOBJECT,
		"DATAVALUE", UA_TYPES_DATAVALUE,
		"VARIANT", UA_TYPES_VARIANT,
		"DIAGNOSTICINFO", UA_TYPES_DIAGNOSTICINFO //TODO:
	);
	module.new_enum("AccessLevel",
		"READ", UA_ACCESSLEVELMASK_READ,
		"WRITE", UA_ACCESSLEVELMASK_WRITE,
		"HISTORYREAD", UA_ACCESSLEVELMASK_HISTORYREAD,
		"HISTORYWRITE", UA_ACCESSLEVELMASK_HISTORYWRITE,
		"SEMANTICCHANGE", UA_ACCESSLEVELMASK_SEMANTICCHANGE,
		"STATUSWRITE", UA_ACCESSLEVELMASK_STATUSWRITE,
		"TIMESTAMPWRITE", UA_ACCESSLEVELMASK_TIMESTAMPWRITE,
		"RW", UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE,
		"ALL", 0xF
	);
	module.new_enum("WriteMask",
		"ACCESSLEVEL", UA_WRITEMASK_ACCESSLEVEL,
		"ARRAYDIMENSIONS", UA_WRITEMASK_ARRRAYDIMENSIONS,
		"BROWSENAME", UA_WRITEMASK_BROWSENAME,
		"CONTAINSNOLOOPS", UA_WRITEMASK_CONTAINSNOLOOPS,
		"DATATYPE", UA_WRITEMASK_DATATYPE,
		"DESCRIPTION", UA_WRITEMASK_DESCRIPTION,
		"DISPLAYNAME", UA_WRITEMASK_DISPLAYNAME,
		"EVENTNOTIFIER", UA_WRITEMASK_EVENTNOTIFIER,
		"EXECUTABLE", UA_WRITEMASK_EXECUTABLE,
		"HISTORIZING", UA_WRITEMASK_HISTORIZING,
		"INVERSENAME", UA_WRITEMASK_INVERSENAME,
		"ISABSTRACT", UA_WRITEMASK_ISABSTRACT,
		"MINIMUMSAMPLINGINTERVAL", UA_WRITEMASK_MINIMUMSAMPLINGINTERVAL,
		"NODECLASS", UA_WRITEMASK_NODECLASS,
		"NODEID", UA_WRITEMASK_NODEID,
		"SYMMETRIC", UA_WRITEMASK_SYMMETRIC,
		"USERACCESSLEVEL", UA_WRITEMASK_USERACCESSLEVEL,
		"USEREXECUTABLE", UA_WRITEMASK_USEREXECUTABLE,
		"USERWRITEMASK", UA_WRITEMASK_USERWRITEMASK,
		"VALUERANK", UA_WRITEMASK_VALUERANK,
		"WRITEMASK", UA_WRITEMASK_WRITEMASK,
		"VALUEFORVARIABLETYPE", UA_WRITEMASK_VALUEFORVARIABLETYPE,
		"ALL", 0xFF
	);

	module.new_enum("LogLevel",
		"TRACE", UA_LogLevel::UA_LOGLEVEL_TRACE,
		"DEBUG", UA_LogLevel::UA_LOGLEVEL_DEBUG,
		"INFO", UA_LogLevel::UA_LOGLEVEL_INFO,
		"WARNING", UA_LogLevel::UA_LOGLEVEL_WARNING,
		"ERROR", UA_LogLevel::UA_LOGLEVEL_ERROR,
		"FATAL", UA_LogLevel::UA_LOGLEVEL_FATAL
	);
	module.new_enum("LogCategory",
		"NETWORK", UA_LogCategory::UA_LOGCATEGORY_NETWORK,
		"SECURECHANNEL", UA_LogCategory::UA_LOGCATEGORY_SECURECHANNEL,
		"SESSION", UA_LogCategory::UA_LOGCATEGORY_SESSION,
		"SERVER", UA_LogCategory::UA_LOGCATEGORY_SERVER,
		"CLIENT", UA_LogCategory::UA_LOGCATEGORY_CLIENT,
		"USERLAND", UA_LogCategory::UA_LOGCATEGORY_USERLAND,
		"SECURITYPOLICY", UA_LogCategory::UA_LOGCATEGORY_SECURITYPOLICY
	);

	module.new_usertype<UA_DateTime>("DateTime",
		//"new", sol::factories([](void) { UA_DateTime date; return UA_DateTime_init(&date); }),
		//"__gc", sol::destructor(UA_DateTime_deleteMembers),
		"new", sol::no_constructor,
		"__tostring", [](const UA_DateTime& date) { return UA_DateTime_toString(date); },
		//"__tostring", [](const UA_DateTime& date) { std::stringstream ss; ss << date; return ss.str(); },
		"toUnixTime", [](const UA_DateTime& date) { return UA_DateTime_toUnixTime(date); },
		"fromUnixTime", [](UA_Int64 unixDate) { return UA_DateTime_fromUnixTime(unixDate); },
		"toStruct", [](const UA_DateTime& date) { return UA_DateTime_toStruct(date); },
		"now", [](void){ return UA_DateTime_now(); },
		"nowMonotonic", [](void){ return UA_DateTime_nowMonotonic(); },
		"localTimeUtcOffset", sol::property([](void) { return UA_DateTime_localTimeUtcOffset(); })
	);
	module.new_usertype<UA_DateTimeStruct>("DateTimeStruct",
		"nanoSec", &UA_DateTimeStruct::nanoSec,
		"microSec", &UA_DateTimeStruct::microSec,
		"milliSec", &UA_DateTimeStruct::milliSec,
		"sec", &UA_DateTimeStruct::sec,
		"min", &UA_DateTimeStruct::min,
		"hour", &UA_DateTimeStruct::hour,
		"day", &UA_DateTimeStruct::day,
		"month", &UA_DateTimeStruct::month,
		"year", &UA_DateTimeStruct::year
	);

	module.new_usertype<UA_Variant>("Variant",
		"new", sol::factories(
			[](bool val) {
				UA_Variant var;
				UA_Variant_init(&var);
				UA_Variant_setScalarCopy(&var, &val, &UA_TYPES[UA_TYPES_BOOLEAN]);
				return var;
			},
			[](std::uint64_t val) {
				//std::cout << "uint64_t" << std::endl;
				UA_Variant var;
				UA_Variant_init(&var);
				UA_Variant_setScalarCopy(&var, &val, &UA_TYPES[UA_TYPES_UINT64]);
				return var;
			},
			[](std::int64_t val) {
				//std::cout << "int64_t" << std::endl;
				UA_Variant var;
				UA_Variant_init(&var);
				UA_Variant_setScalarCopy(&var, &val, &UA_TYPES[UA_TYPES_INT64]);
				return var;
			},
			[](std::uint32_t val, bool v32) {
				//std::cout << "uint32_t" << std::endl;
				UA_Variant var;
				UA_Variant_init(&var);
				UA_Variant_setScalarCopy(&var, &val, &UA_TYPES[UA_TYPES_UINT32]);
				return var;
			},
			[](std::int32_t val, bool v32) {
				//std::cout << "int32_t" << std::endl;
				UA_Variant var;
				UA_Variant_init(&var);
				UA_Variant_setScalarCopy(&var, &val, &UA_TYPES[UA_TYPES_INT32]);
				return var;
			},
			[](double val) {
				UA_Variant var;
				UA_Variant_init(&var);
				UA_Variant_setScalarCopy(&var, &val, &UA_TYPES[UA_TYPES_DOUBLE]);
				return var;
			},
			[](const char* val) {
				UA_String str = UA_STRING_ALLOC(val);
				UA_Variant var;
				UA_Variant_init(&var);
				UA_Variant_setScalarCopy(&var, &str, &UA_TYPES[UA_TYPES_STRING]);
				UA_String_deleteMembers(&str);
				return var;
			},
			[](const UA_Variant& obj) {
				UA_Variant var;
				UA_Variant_init(&var);
				UA_Variant_copy(&obj, &var);
				return var;
			},
			[]() {
				UA_Variant var;
				UA_Variant_init(&var);
				return var;
			}
		),
		"__gc", sol::destructor(UA_Variant_deleteMembers),
		"isEmpty", [](UA_Variant& var) { return UA_Variant_isEmpty(&var); },
		"isScalar", [](UA_Variant& var) { return UA_Variant_isScalar(&var); },
		"hasScalarType", [](UA_Variant& var, int type) { return UA_Variant_hasScalarType(&var, &UA_TYPES[type]); },
		"hasArrayType", [](UA_Variant& var, int type) { return UA_Variant_hasArrayType(&var, &UA_TYPES[type]); },
		"setScalar", [](UA_Variant& var, void* p, int type) { return UA_Variant_setScalar(&var, p, &UA_TYPES[type]); },
		"setScalarCopy", [](UA_Variant& var, void* p, int type) { return UA_Variant_setScalarCopy(&var, p, &UA_TYPES[type]); },
		"setArray", [](UA_Variant& var, void* array, size_t arraySize, int type) { return UA_Variant_setArray(&var, array, arraySize, &UA_TYPES[type]); },
		"setArrayCopy", [](UA_Variant& var, void* array, size_t arraySize, int type) { return UA_Variant_setArrayCopy(&var, array, arraySize, &UA_TYPES[type]); },
		"copyRange", [](UA_Variant& var, const UA_Variant& src, const UA_NumericRange range) { return UA_Variant_copyRange(&src, &var, range); },
		"setRange", [](UA_Variant& var, void* array, size_t arraySize, const UA_NumericRange range) { return UA_Variant_setRange(&var, array, arraySize, range); },
		"setRangeCopy", [](UA_Variant& var, void* array, size_t arraySize, const UA_NumericRange range) { return UA_Variant_setRangeCopy(&var, array, arraySize, range); }
	);

	module.new_usertype<UA_DataValue>("DataValue",
		"new", sol::factories(
			[](const UA_Variant& val) {
				UA_DataValue var;
				UA_DataValue_init(&var);
				if (!UA_Variant_isEmpty(&val))
					var.hasValue = true;
				UA_Variant_copy(&val, &var.value);
				return var;
			}
		),
		"__gc", sol::destructor(UA_DataValue_deleteMembers),
		"hasValue", sol::property([](UA_DataValue& obj) { return obj.hasValue; }),
		"hasStatus", sol::property([](UA_DataValue& obj) { return obj.hasStatus; }),
		"hasSourceTimestamp", sol::property([](UA_DataValue& obj) { return obj.hasSourceTimestamp; }),
		"hasServerTimestamp", sol::property([](UA_DataValue& obj) { return obj.hasServerTimestamp; }),
		"hasSourcePicoseconds", sol::property([](UA_DataValue& obj) { return obj.hasSourcePicoseconds; }),
		"hasServerPicoseconds", sol::property([](UA_DataValue& obj) { return obj.hasServerPicoseconds; }),
		MAP_PROPERTY_HAS(UA_DataValue, UA_Variant, value, hasValue),
		"status", sol::property(
			[](UA_DataValue& obj) { return obj.status; },
			[](UA_DataValue& obj, UA_StatusCode status) { obj.status = status, obj.hasStatus = true; }
		),
		MAP_PROPERTY_HAS(UA_DataValue, UA_DateTime, sourceTimestamp, hasSourceTimestamp),
		"sourcePicoseconds", sol::property(
			[](UA_DataValue& obj) { return obj.sourcePicoseconds; },
			[](UA_DataValue& obj, UA_UInt16 seconds) { obj.sourcePicoseconds = seconds, obj.hasSourcePicoseconds = true; }
		),
		MAP_PROPERTY_HAS(UA_DataValue, UA_DateTime, serverTimestamp, hasServerTimestamp),
		"serverPicoseconds", sol::property(
			[](UA_DataValue& obj) { return obj.serverPicoseconds; },
			[](UA_DataValue& obj, UA_UInt16 seconds) { obj.serverPicoseconds = seconds, obj.hasServerPicoseconds = true; }
		)
	),

	module.new_usertype<UA_NodeId>("NodeId",
		"new", sol::factories(
			[](int ns, int val) { return UA_NODEID_NUMERIC(ns, val); },
			[](int ns, const char* val){ return UA_NODEID_STRING_ALLOC(ns, val); },
			[](int ns, UA_Guid val){ UA_NODEID_GUID(ns, val); }
		),
		"__gc", sol::destructor(UA_NodeId_deleteMembers),
		"__eq", [](const UA_NodeId& left, const UA_NodeId& right) { return UA_NodeId_equal(&left, &right); },
		"isNull", [](const UA_NodeId& id) { return UA_NodeId_isNull(&id); },
		"hash", [](const UA_NodeId& id) { return UA_NodeId_hash(&id); },
		"ns", &UA_NodeId::namespaceIndex,
		"type", &UA_NodeId::identifierType,
		"index", sol::property([](UA_NodeId& id, sol::this_state L) {
			sol::variadic_results result;
			switch (id.identifierType) {
			case UA_NODEIDTYPE_NUMERIC:
				result.push_back({ L, sol::in_place_type<int>, id.identifier.numeric});
				break;
			case UA_NODEIDTYPE_STRING:
				result.push_back({ L, sol::in_place_type<const std::string>, std::string((char*)id.identifier.string.data, id.identifier.string.length)});
				break;
			case UA_NODEIDTYPE_GUID:
				result.push_back({ L, sol::in_place_type<UA_Guid>, id.identifier.guid});
				break;
			case UA_NODEIDTYPE_BYTESTRING:
				result.push_back({ L, sol::in_place_type<const std::string>, std::string((char*)id.identifier.byteString.data, id.identifier.byteString.length)});
				break;
			default:
				result.push_back({ L, sol::in_place_type<sol::lua_nil_t>, sol::lua_nil_t()});
				break;
			}
			return result;
		})
	);
	module.new_usertype<UA_QualifiedName>("QualifiedName",
		"new", sol::factories([](int ns, const char* val) { return UA_QUALIFIEDNAME_ALLOC(ns, val); }),
		"__gc", sol::destructor(UA_QualifiedName_deleteMembers),
		"__eq", [](const UA_QualifiedName& left, const UA_QualifiedName& right) { return left.namespaceIndex == right.namespaceIndex & UA_String_equal(&left.name, &right.name); }
	);

	module.new_usertype<UA_LocalizedText>("LocalizedText",
		"new", sol::factories([](const char* locale, const char* text) { return UA_LOCALIZEDTEXT_ALLOC(locale, text); }),
		"__gc", sol::destructor(UA_LocalizedText_deleteMembers)
	);

	module.new_usertype<UA_ObjectAttributes>("ObjectAttributes",
		"new", sol::factories([]() { 
			return UA_ObjectAttributes_default;
		}),
		"__gc", sol::destructor(UA_ObjectAttributes_deleteMembers),
		"specifiedAttributes", &UA_ObjectAttributes::specifiedAttributes,
		MAP_PROPERTY(UA_ObjectAttributes, UA_LocalizedText, displayName),
		MAP_PROPERTY(UA_ObjectAttributes, UA_LocalizedText, description),
		"writeMask", &UA_ObjectAttributes::writeMask,
		"userWriteMask", &UA_ObjectAttributes::userWriteMask,
		"eventNotifier", &UA_ObjectAttributes::eventNotifier
	);

	module.new_usertype<UA_VariableAttributes>("VariableAttributes",
		"new", sol::factories([]() { 
			return UA_VariableAttributes_default;
		}),
		"__gc", sol::destructor(UA_VariableAttributes_deleteMembers),
		"specifiedAttributes", &UA_VariableAttributes::specifiedAttributes,
		MAP_PROPERTY(UA_VariableAttributes, UA_LocalizedText, displayName),
		MAP_PROPERTY(UA_VariableAttributes, UA_LocalizedText, description),
		"writeMask", &UA_VariableAttributes::writeMask,
		"userWriteMask", &UA_VariableAttributes::userWriteMask,
		MAP_PROPERTY(UA_VariableAttributes, UA_Variant, value),
		MAP_PROPERTY(UA_VariableAttributes, UA_NodeId, dataType),
		"valueRank", &UA_VariableAttributes::valueRank,
		"arrayDimensionsSize", &UA_VariableAttributes::arrayDimensionsSize,
		"accessLevel", &UA_VariableAttributes::accessLevel,
		"userAccessLevel", &UA_VariableAttributes::userAccessLevel,
		"minimumSamplingInterval", &UA_VariableAttributes::minimumSamplingInterval,
		"historizing", &UA_VariableAttributes::historizing
	);

	module.new_usertype<UA_ViewAttributes>("ViewAttributes",
		"new", sol::factories([]() { 
			return UA_ViewAttributes_default;
		}),
		"__gc", sol::destructor(UA_ViewAttributes_deleteMembers),
		"specifiedAttributes", &UA_ViewAttributes::specifiedAttributes,
		MAP_PROPERTY(UA_ViewAttributes, UA_LocalizedText, displayName),
		MAP_PROPERTY(UA_ViewAttributes, UA_LocalizedText, description),
		"writeMask", &UA_ViewAttributes::writeMask,
		"userWriteMask", &UA_ViewAttributes::userWriteMask,
		"containsNoLoops", &UA_ViewAttributes::containsNoLoops,
		"eventNotifier", &UA_ViewAttributes::eventNotifier
	);

	module.new_usertype<UA_MethodAttributes>("MethodAttributes",
		"new", sol::factories([]() { 
			return UA_MethodAttributes_default;
		}),
		"__gc", sol::destructor(UA_MethodAttributes_deleteMembers),
		"specifiedAttributes", &UA_MethodAttributes::specifiedAttributes,
		MAP_PROPERTY(UA_MethodAttributes, UA_LocalizedText, displayName),
		MAP_PROPERTY(UA_MethodAttributes, UA_LocalizedText, description),
		"writeMask", &UA_MethodAttributes::writeMask,
		"userWriteMask", &UA_MethodAttributes::userWriteMask,
		"executable", &UA_MethodAttributes::executable,
		"userExecutable", &UA_MethodAttributes::userExecutable
	);
}

}
