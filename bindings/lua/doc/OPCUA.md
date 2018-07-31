# OpcUa(open61850) Lua binding

## Methods

* setLogger
Set logger callback method

``` lua
local opcua = require 'opcua'
opcua.setLogger(function(level, category, msg)
	print(level, category, msg)
end)
```

* getStatusCodeName
Get cosponding string name of UA_StatusCode enums

```
local r = client:connect()
print(opcua.getStatusCodeName(r))
```
