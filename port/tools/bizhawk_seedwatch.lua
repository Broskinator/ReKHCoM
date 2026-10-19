-- Original cartridge (USA): plays the speedrun movie and logs every room the
-- game creates (gMapFloorState.rooms[i] at 0x0203C5AC + 16*i: flags, seed,
-- nameId, roomType) with the frame and gFrameCounter (0x03007480), so the
-- first save-crystal room can be reproduced at different moments. With
-- KHCOM_BH_DELAY=N and KHCOM_BH_AT=F it inserts N idle frames before movie
-- frame F (the movie's inputs then resume), to enter the same door later.
local path = os.getenv("KHCOM_BH_VBM")
local out = os.getenv("KHCOM_BH_OUT") or "."
local stop = tonumber(os.getenv("KHCOM_BH_STOP") or "20000")
local delay = tonumber(os.getenv("KHCOM_BH_DELAY") or "0")
local at = tonumber(os.getenv("KHCOM_BH_AT") or "-1")
local shots = os.getenv("KHCOM_BH_SHOTS") or ""
local f = assert(io.open(path, "rb"))
local data = f:read("*a")
f:close()
local start = string.unpack("<I4", data, 0x3C + 1)
local names = { "A", "B", "Select", "Start", "Right", "Left", "Up", "Down", "R", "L" }
local log = io.open(out .. "/seedwatch.log", "w")
local seen = {}
pcall(client.speedmode, 6400)
local i, waited = 0, 0
local real = 0
while i < stop do
    local pad = {}
    if i == at and waited < delay then
        waited = waited + 1
    else
        local v = string.unpack("<I2", data, start + i * 2 + 1)
        for b = 0, 9 do
            if v & (1 << b) ~= 0 then pad[names[b + 1]] = true end
        end
        i = i + 1
    end
    joypad.set(pad)
    emu.frameadvance()
    real = real + 1
    for r = 0, 31 do
        local base = 0x3C5AC + r * 16
        local seed = memory.read_u32_le(base + 4, "EWRAM")
        local key = r .. ":" .. seed
        if seed ~= 0 and not seen[key] then
            seen[key] = true
            local fc = memory.read_u32_le(0x7480, "IWRAM")
            log:write(string.format("movie=%d real=%d room=%d seed=%08x type=%d fc=%d\n", i, real, r, seed,
                memory.read_u8(base + 9, "EWRAM"), fc))
            log:flush()
        end
    end
    for s in shots:gmatch("%d+") do
        if tonumber(s) == real then client.screenshot(string.format("%s/sw_%06d.png", out, real)) end
    end
end
log:close()
client.exit()
