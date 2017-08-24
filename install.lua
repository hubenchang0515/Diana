#! /usr/bin/env lua

local cpath = string.gsub(package.cpath,"%?.*","",1)
os.execute("cp diana.so " .. cpath);
