-- Original cartridge (USA): which instruction writes room 1's seed
-- (gMapFloorState.rooms[1].seed, 0x0203C5C0) while the speedrun movie plays.
-- Logs PC, LR, the value and the frame counter (0x03007480) of each write.
local path = os.getenv("KHCOM_BH_VBM")
local out = os.getenv("KHCOM_BH_OUT") or "."
local stop = tonumber(os.getenv("KHCOM_BH_STOP") or "7430")
local f = assert(io.open(path, "rb"))
local data = f:read("*a")
f:close()
local start = string.unpack("<I4", data, 0x3C + 1)
local names = { "A", "B", "Select", "Start", "Right", "Left", "Up", "Down", "R", "L" }
local log = io.open(out .. "/seedtrace.log", "w")
local frame = 0
event.onmemorywrite(function(addr, val, flags)
    log:write(string.format("frame=%d addr=%08x val=%08x pc=%08x lr=%08x fc=%d seed=%08x\n", frame,
        addr or 0, val or 0, emu.getregister("R15") or 0, emu.getregister("R14") or 0,
        memory.read_u32_le(0x7480, "IWRAM"), memory.read_u32_le(0x3C5C0, "EWRAM")))
    log:flush()
end, 0x0203C5C0, "seedtrace", "System Bus")
pcall(client.speedmode, 6400)
for i = 0, stop - 1 do
    frame = i
    local v = string.unpack("<I2", data, start + i * 2 + 1)
    local pad = {}
    for b = 0, 9 do
        if v & (1 << b) ~= 0 then pad[names[b + 1]] = true end
    end
    joypad.set(pad)
    emu.frameadvance()
end
log:close()
client.exit()
