-- ldoc configuration file
project = "open62541 lua wrapper"
title = "Open62541 Lua wrapper documentation"
description = "A binding to the Open62541 (OpcUA)."
examples = {"test", exclude = {"test/test.lua"}}
readme = { 
	"README.md", 
	"doc/TYPES.md", 
	"doc/ENUMS.md" ,
	"doc/NODE.md",
	"doc/CLIENT.md",
	"doc/SERVER.md",
}
format = "discount"
dir = "ldoc"
file = "src"
wrap = true
add_language_extension("hpp", "c")
