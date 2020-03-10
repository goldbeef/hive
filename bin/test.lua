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
            print("******************")
            for k2, v2 in pairs(v) do
                print(k .. ".k.v", k2, v2)
            end
            print("******************")
        end
        print("++++++++++++++++++++++++++++")
    end

    --os.execute("sleep 3") --can not sigint
end


