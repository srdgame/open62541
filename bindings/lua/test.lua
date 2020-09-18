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

--local client = opcua.Client.new("opc.tcp://10.0.0.117:49320", 5000, 10 * 60 * 1000, config, "Basic128Rsa15", "certs/my_cert.der", "certs/my_private_key.der", "")
local client = opcua.Client.new("opc.tcp://10.0.0.117:49320", 5000, 10 * 60 * 1000, config, "", "certs/my_cert.der", "certs/my_private_key.der", "")
--local client = opcua.Client.new("opc.tcp://127.0.0.1:4840", 5000, 10 * 60 * 1000, config, "Basic128Rsa15", "certs/my_cert.der", "certs/my_private_key.der", "")
print(client)
local r, err = client:connect_username("Administrator", "Pa88word")
--local r, err = client:connect()
--local r, err = client:connect("opc.tcp://Administrator:Pa88word@172.30.1.153:49320")
--local r, err = client:connect("opc.tcp://172.30.1.153:49320")
print('connect result', r, err)
if r ~= 0 then
	print(opcua.getStatusCodeName(r))
end

local root = client:getRootNode()
print("Root:", root, root.browse_name)
print("Server:", client:getServerNode())

print (client:getNamespaceIndex("KEPServerEX"))

os.execute("sleep 10")
