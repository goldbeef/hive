#!./hive

require("base/log");
require("base/tree");

ct = ct or 0;

function hive.run()
    hive.default_signal(2)
    --hive.ignore_signal(2)
    print("ct="..ct);
	log_debug("ct=%d", ct);
    ct = ct + 1;
	if ct > 10000 then
		print("quit");
		hive.run = nil;
	end

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

    --os.execute("sleep 10") --can not sigint
    hive.sleep_ms(3000)
end


