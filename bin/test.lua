#!/usr/bin/hive

import("base/log.lua");

ct = ct or 0;

function hive.run()
    hive.sleep_ms(1000);

    print("ct="..ct);
	log_debug("ct=%d", ct);
    ct = ct + 1;
	if ct > 10000 then
		print("quit");
		hive.run = nil;
	end
end


