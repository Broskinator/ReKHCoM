-- Field position probe on the original cartridge (BizHawk, GBA core).
--
-- Plays a VBA-rr movie (.vbm) up to KHCOM_BH_STOP, logging the u32 words at
-- KHCOM_BH_ADDR (the player's FldPos x, y, z in EWRAM) every KHCOM_BH_EVERY
-- frames with a screenshot. If KHCOM_BH_SET="x,y,z" is given, it then writes
-- that position (the PC port's value from a savestate) and holds the buttons of
-- KHCOM_BH_HOLD ("Left" or "Up+Left"...) for KHCOM_BH_HOLDFRAMES frames,
-- logging the position each 4 frames: the same test the port's KHCOM_POSLOG
-- runs, so both can be compared. Run through port/tools/bizhawk_probe.py.

local path = os.getenv("KHCOM_BH_VBM")
local outdir = os.getenv("KHCOM_BH_OUT") or "."
local every = tonumber(os.getenv("KHCOM_BH_EVERY") or "30")
local stop = tonumber(os.getenv("KHCOM_BH_STOP") or "6000")
local addr = tonumber(os.getenv("KHCOM_BH_ADDR") or "0x02033EF8")
local set = os.getenv("KHCOM_BH_SET")
local hold = os.getenv("KHCOM_BH_HOLD") or "Left"
local holdFrames = tonumber(os.getenv("KHCOM_BH_HOLDFRAMES") or "60")

local f = assert(io.open(path, "rb"))
local data = f:read("*a")
f:close()
local start = string.unpack("<I4", data, 0x3C + 1)
local names = { "A", "B", "Select", "Start", "Right", "Left", "Up", "Down", "R", "L" }
local log = io.open(outdir .. "/probe.log", "w")

local function pos()
    local x = memory.read_s32_le(addr - 0x02000000, "EWRAM")
    local y = memory.read_s32_le(addr - 0x02000000 + 4, "EWRAM")
    local z = memory.read_s32_le(addr - 0x02000000 + 8, "EWRAM")
    return x, y, z
end

pcall(client.speedmode, 6400)
while emu.framecount() < stop do
    local i = emu.framecount()
    local v = string.unpack("<I2", data, start + i * 2 + 1)
    local pad = {}
    for b = 0, 9 do
        if v & (1 << b) ~= 0 then pad[names[b + 1]] = true end
    end
    joypad.set(pad)
    emu.frameadvance()
    local n = emu.framecount()
    if n % every == 0 then
        local x, y, z = pos()
        log:write(string.format("movie f=%d x=%d y=%d z=%d\n", n, x, y, z))
        log:flush()
        client.screenshot(string.format("%s/bh_%06d.png", outdir, n))
    end
end

if set then
    local sx, sy, sz = set:match("(-?%d+),(-?%d+),(-?%d+)")
    for k = 1, holdFrames do
        if k <= 2 then
            memory.write_s32_le(addr - 0x02000000, tonumber(sx), "EWRAM")
            memory.write_s32_le(addr - 0x02000000 + 4, tonumber(sy), "EWRAM")
            memory.write_s32_le(addr - 0x02000000 + 8, tonumber(sz), "EWRAM")
        end
        local pad = {}
        if k > 4 then
            for b in hold:gmatch("[^+]+") do pad[b] = true end
        end
        joypad.set(pad)
        emu.frameadvance()
        if k % 4 == 0 then
            local x, y, z = pos()
            log:write(string.format("hold %s k=%d x=%d y=%d z=%d\n", hold, k, x, y, z))
            log:flush()
        end
    end
    client.screenshot(outdir .. "/bh_after.png")
end
log:close()
client.exit()
