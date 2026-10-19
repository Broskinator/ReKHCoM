-- Load quick-save slot KHCOM_BH_SLOT, advance a couple of frames, then save a
-- screenshot and the whole EWRAM and IWRAM to KHCOM_BH_OUT, and close.
local slot = tonumber(os.getenv("KHCOM_BH_SLOT") or "1")
local out = os.getenv("KHCOM_BH_OUT") or "."
savestate.loadslot(slot)
emu.frameadvance()
emu.frameadvance()
client.screenshot(out .. "/snapshot.png")
for _, d in ipairs({ { "EWRAM", 0x40000 }, { "IWRAM", 0x8000 } }) do
    local f = io.open(out .. "/" .. d[1] .. ".bin", "wb")
    local bytes = memory.read_bytes_as_array(0, d[2], d[1])
    local chunk = {}
    for i = 1, #bytes do
        chunk[#chunk + 1] = string.char(bytes[i])
        if #chunk == 4096 then f:write(table.concat(chunk)); chunk = {} end
    end
    f:write(table.concat(chunk))
    f:close()
end
client.exit()
