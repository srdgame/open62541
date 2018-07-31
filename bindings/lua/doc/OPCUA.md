# OpcUa(open61850) Lua binding

## Functions

* [setLogger](#setLogger)
* [getStatusCodeName](#getStatusCodeName)

##### setLogger
> Set logger callback method

``` lua
local opcua = require 'opcua'
opcua.setLogger(function(level, category, msg)
	print(level, category, msg)
end)
```

##### getStatusCodeName

> Get cosponding string name of UA_StatusCode enums

``` lua
local r = client:connect()
print(opcua.getStatusCodeName(r))
```
