-- Play a VBA-rr input movie (.vbm) in BizHawk's GBA core and save a
-- screenshot every KHCOM_BH_EVERY frames, for comparison with the PC port
-- playing the same movie (KHCOM_VBM). Run through port/tools/bizhawk_vbm.py.

local path = os.getenv("KHCOM_BH_VBM")
local outdir = os.getenv("KHCOM_BH_OUT") or "."
local every = tonumber(os.getenv("KHCOM_BH_EVERY") or "600")
local total = tonumber(os.getenv("KHCOM_BH_FRAMES") or "0")

local f = assert(io.open(path, "rb"))
local data = f:read("*a")
f:close()
local start = string.unpack("<I4", data, 0x3C + 1)
local frames = (#data - start) // 2
if total == 0 or total > frames then total = frames end

local names = { "A", "B", "Select", "Start", "Right", "Left", "Up", "Down", "R", "L" }

pcall(client.speedmode, tonumber(os.getenv("KHCOM_BH_SPEED") or "6400"))
while emu.framecount() < total do
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
        client.screenshot(string.format("%s/bh_%06d.png", outdir, n))
    end
end
client.exit()
