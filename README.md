# Diana
Diana is a Lua utility library , has IO SOCKET etc  
*Hasn't been ready*

## Demo
```Lua
#! /usr/bin/env lua
local socket = require("diana.socket")
local signal = require("diana.signal")

local listen = socket.tcp()

signal.set(signal.SIGINT, function() listen:close() print("Interrupt") end)

if listen:bind('127.0.0.1',80) and listen:listen() then
	while true do
		local x = listen:accept()
		if x == nil then
			break
		end
		x:write("HTTP/1.1 200 OK\n")
		x:write("Content-Type: text/html\n\n")
		x:write("<h1>Hello Diana</h1>")
		x:close()
	end
else
	print("Bind Failed")
	listen:close()
end
```
