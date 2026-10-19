-- Original cartridge experiment (issue 63, 19/10): does the save crystal's spot
-- come from the room seed? Loads quick-save slot KHCOM_BH_SLOT, writes the seed
-- KHCOM_BH_SEED into the floor state's current room (and its heap copy), then
-- plays KHCOM_BH_KEYS ("from-to:Button+Button,...", frames after loading) so the
-- game saves at the crystal and reloads the save, which rebuilds the room from
-- that seed. Screenshots every KHCOM_BH_EVERY frames to KHCOM_BH_OUT; closes at
-- KHCOM_BH_END. Addresses are the USA cartridge's (dump of 19/10).
local slot = tonumber(os.getenv("KHCOM_BH_SLOT") or "1")
local seed = tonumber(os.getenv("KHCOM_BH_SEED") or "0")
local keys = os.getenv("KHCOM_BH_KEYS") or ""
local every = tonumber(os.getenv("KHCOM_BH_EVERY") or "60")
local finish = tonumber(os.getenv("KHCOM_BH_END") or "600")
local out = os.getenv("KHCOM_BH_OUT") or "."
pcall(client.speedmode, 800)
savestate.loadslot(slot)
if seed ~= 0 then
    memory.write_u32_le(0x0003C5C0, seed, "EWRAM")
    memory.write_u32_le(0x00017768, seed, "EWRAM")
end
local plan = {}
for from, to, buttons in keys:gmatch("(%d+)-(%d+):([%w%+]+)") do
    plan[#plan + 1] = { tonumber(from), tonumber(to), buttons }
end
for f = 1, finish do
    local pad = {}
    for _, p in ipairs(plan) do
        if f >= p[1] and f <= p[2] then
            for b in p[3]:gmatch("[^+]+") do pad[b] = true end
        end
    end
    joypad.set(pad)
    emu.frameadvance()
    if f % every == 0 then client.screenshot(string.format("%s/rs_%05d.png", out, f)) end
end
local f = io.open(out .. "/rs_ewram.bin", "wb")
local bytes = memory.read_bytes_as_array(0, 0x40000, "EWRAM")
local chunk = {}
for i = 1, #bytes do
    chunk[#chunk + 1] = string.char(bytes[i])
    if #chunk == 4096 then f:write(table.concat(chunk)); chunk = {} end
end
f:write(table.concat(chunk)); f:close()
client.exit()
