#!./hive

--require("base/log");
--require("base/tree");

import("base/log.lua")
import("base/tree.lua")

ct = ct or 0;

function hive.run()
    hive.register_signal(2)
    --hive.default_signal(2)
    --hive.ignore_signal(2)
    print("ct="..ct);
	log_debug("ct=%d", ct);
    ct = ct + 1;
	if ct > 10000 then
		print("quit");
		hive.run = nil;
	end

	print("signal: " .. hive.signal)

	print("---------------hive table.....")
    for k, v in pairs(hive) do
        print("++++++++++++++++++++++++++++")
        print(k, v, type(k), type(v))
        if type(v) == "table" then
            for k2, v2 in pairs(v) do
                print("\t k.v", k2, v2)
                if type(v2) == "table" then
                    for k3, v3 in pairs(v2) do
                        print("\t\t k.v", k3, v3)
                    end
                end
            end
        end
    end

	print("--------------hive.meta table.....")
    hiveMeta = getmetatable(hive)
    for k, v in pairs(hiveMeta) do
        print("meta.k.v", k, v, type(k), type(v))
    end

    print(hive.get_full_path("test"))
    --local withroot = nil and hive.get_full_path(rootpath).."/"..filename or filename;
    --local fullpath = hive.get_full_path(withroot) or withroot;
    --os.execute("sleep 10") --can not sigint
    hive.sleep_ms(3000)
end


