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

    print("--------------------------------------------------")
    for k, v in pairs(hive) do
        print(k, v, type(k), type(v))
        if type(v) == "table" then
            print("**********************************")
            for k2, v2 in pairs(v) do
                print(k, k2, v2, type(k2), type(v2))
            end
        end
    end
    print("--------------------------------------------------")

    os.execute("sleep 3")
end


