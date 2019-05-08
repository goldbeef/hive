#!./hive

require("base/log");
require("base/tree");

ct = ct or 0;

function hive.run()
    print("ct="..ct);
	log_debug("ct=%d", ct);
    ct = ct + 1;
	if ct > 10000 then
		print("quit");
		hive.run = nil;
	end
end


