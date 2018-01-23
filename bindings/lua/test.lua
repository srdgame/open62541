local opcua = require 'opcua'
--[[
opcua.setLogger(function(...)
	print(...)
end)
]]--

local config = opcua.ConnectionConfig.new()
config.protocolVersion = 0
config.sendBufferSize = 65535
config.recvBufferSize = 65535
config.maxMessageSize = 0
config.maxChunkCount = 0

local client = opcua.Client.new(5000, 10 * 60 * 1000, config)
local r, err = client:connect_username("opc.tcp://172.30.1.153:49320", "Administrator", "Pa88word")
--local r, err = client:connect("opc.tcp://Administrator:Pa88word@172.30.1.153:49320")
--local r, err = client:connect("opc.tcp://172.30.1.153:49320")
print(r, err)

local root = client:getRootNode()
print("Root:", root, root.browse_name)
print("Server:", client:getServerNode())

print (client:getNamespaceIndex("KEPServerEX"))

os.execute("sleep 10")
