local tap = {}
local counter = 1

function tap.ok(assert_true, desc)
   local msg = ( assert_true and "ok " or "not ok " ) .. counter
   if ( desc ) then
      msg = msg .. " - " .. desc
   end
   print(msg)
   counter = counter + 1
end

return tap
