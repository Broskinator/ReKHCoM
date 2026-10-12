-- Measure the cartridge's own pacing on hardware-accurate timing (BizHawk,
-- mGBA core). Every 300 frames (VBlanks) it logs how many main-loop updates
-- the game made (gFrameCounter) and whether a movie was playing. Input follows
-- the same Spanish new-game route as the PC test harness.
--
-- Run through port/tools/bizhawk_timing.py, which sets KHCOM_BH_OUT and
-- KHCOM_BH_FRAMES.

local FRAME_COUNTER = 0x03007480   -- gFrameCounter: main-loop updates (EU)
local VBLANK_COUNTER = 0x03006C68  -- gVBlankCounter
local VBLANK_OVERRIDE = 0x03007488 -- gVBlankHandlerOverride (set by movies)

local out = io.open(os.getenv("KHCOM_BH_OUT") or "bizhawk_timing.log", "w")
local total = tonumber(os.getenv("KHCOM_BH_FRAMES") or "26000")

-- frame ranges -> buttons, like KHCOM_KEYS
local schedule = {
    {150, 152, "Down"}, {165, 167, "Down"}, {180, 182, "Down"}, {200, 204, "A"},
    {1260, 1264, "Start"}, {1400, 1404, "A"}, {1550, 1554, "A"},
}
for f = 1700, total, 50 do
    schedule[#schedule + 1] = {f, f + 3, "A"}
end

local function bus32(addr)
    return memory.read_u32_le(addr, "System Bus")
end

-- Report failures in the log instead of stopping silently in the console.
local function try(name, fn, ...)
    local ok, err = pcall(fn, ...)
    if not ok then
        out:write("# " .. name .. " failed: " .. tostring(err) .. "\n")
        out:flush()
    end
    return ok
end

out:write("# frame game_vblanks updates_in_window movie\n")
out:flush()
try("speedmode", client.speedmode, tonumber(os.getenv("KHCOM_BH_SPEED") or "6400"))

local lastUpdates = bus32(FRAME_COUNTER)
while emu.framecount() < total do
    local f = emu.framecount()
    local pad = {}
    for _, s in ipairs(schedule) do
        if f >= s[1] and f <= s[2] then
            pad[s[3]] = true
        end
    end
    if not try("joypad.set", joypad.set, pad) then break end
    emu.frameadvance()

    f = emu.framecount()
    if f % 300 == 0 then
        local updates = bus32(FRAME_COUNTER)
        local movie = bus32(VBLANK_OVERRIDE) ~= 0
        out:write(string.format("%6d %8d %4d %s\n", f, bus32(VBLANK_COUNTER),
            (updates - lastUpdates) % 0x100000000, movie and "movie" or "-"))
        out:flush()
        lastUpdates = updates
    end
end

out:close()
client.exit()
