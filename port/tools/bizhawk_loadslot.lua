-- Open BizHawk on quick-save slot KHCOM_BH_SLOT (default 1) at normal speed and
-- leave the player in control: the script only loads the state.
local slot = tonumber(os.getenv("KHCOM_BH_SLOT") or "1")
pcall(client.speedmode, 100)
savestate.loadslot(slot)
