print "1..6"

local tap   = require("tap")
local yajl  = require("yajl")
local ok    = tap.ok

function main()
   test_simple()
end

function test_simple()
   local expect =
      '{'..
      '"float":1.5,'..
      '"integer":5,'..
      '"true":true,'..
      '"false":false,'..
      '"null":null,'..
      '"string":"hello",'..
      '"array":[],'..
      '"object":{},'..
      '}'

   -- Input to deflate is same as output to inflate:
   local got = yajl.to_string(yajl.to_value(expect))
   ok(expect == got, expect .. " == " .. tostring(got))
end

main()