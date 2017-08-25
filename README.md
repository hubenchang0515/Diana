# Diana
Diana is a Lua utility library , has IO SOCKET etc  
*Hasn't been ready*

## Demo
```Lua
#! /usr/bin/env lua
local socket = require("diana.socket")
local signal = require("diana.signal")

local listen = socket.tcp()

signal.set(signal.SIGINT, function() listen:close() os.exit()  end)

if listen:bind('0.0.0.0',80) and listen:listen() then
	while true do
		local x,addr = listen:accept()
		if x == nil then
			break
		end

		print(addr)
		x:write("HTTP/1.1 200 OK\n")
		x:write("Content-Type: text/html\n\n")
		x:write("<h1>Hello Diana</h1>")
		x:close()
	end
else
	print(listen:error())
	listen:close()
end
```
