#!./hive

require("base/log");
require("base/tree");
lbus = require("lbus");

mgr = mgr or lbus.create_socket_mgr(1000);

ct = ct or 0;

function hive.run()
    mgr.wait(1000);

    print("ct="..ct);
	log_debug("ct=%d", ct);
    ct = ct + 1;
	if ct > 10000 then
		print("quit");
		hive.run = nil;
	end
end


