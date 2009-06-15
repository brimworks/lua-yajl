print "1..6"

local tap   = require("tap")
local yajl  = require("yajl")
local ok    = tap.ok

function main()
   test_simple()
   nil_at_end_of_array();
end

function to_value(string)
   local result
   local stack = {
      function(val) result = val end
   }
   local obj_key
   local events = {
      value = function(_, val)
                 stack[#stack](val)
              end,
      open_array = function()
                      local arr = {}
                      local idx = 1
                      stack[#stack](arr)
                      table.insert(stack, function(val)
                                             arr[idx] = val
                                             idx = idx + 1
                                          end)
                   end,
      open_object = function()
                      local obj = {}
                      stack[#stack](obj)
                      table.insert(stack, function(val)
                                             obj[obj_key] = val
                                          end)
                   end,
      object_key = function(_, val)
                     obj_key = val
                  end,
      close = function()
                stack[#stack] = nil
             end,
   }

   yajl.parser({ events = events })(string)
   return result
end

function test_simple()
   local expect =
      '['..
      '"float",1.5,'..
      '"integer",5,'..
      '"true",true,'..
      '"false",false,'..
      '"null",null,'..
      '"string","hello",'..
      '"array",[],'..
      '"object",{"key":"value"}'..
      ']'

   -- Input to deflate is same as output to inflate:
   local got = yajl.to_string(to_value(expect))
   ok(expect == got, expect .. " == " .. tostring(got))
end

function nil_at_end_of_array()
   -- TODO: How to represent the fact that nil can not be stored in a table!?
   local expect = '["something",null]'
   local got = yajl.to_string(to_value(expect))
   ok(expect == got, expect .. " == " .. tostring(got))
end

main()