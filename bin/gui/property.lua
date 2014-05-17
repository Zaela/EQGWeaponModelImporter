
local button = iup.button{title = "Add Property", padding = "31x0"}

local options = {
	e_TextureDiffuse0 = 2,
	e_TextureNormal0 = 2,
	e_TextureEnvironment0 = 2,
	e_fShininess0 = 0,
	e_fBumpiness0 = 0,
	e_fEnvMapStrength0 = 0,
}

function button:action()
	local m = cur_material
	if not m then return end
	local list = iup.list{dropdown = "YES", visiblecolumns = 16}
	local opt = {}
	for o in pairs(options) do
		opt[o] = true
	end
	for _, p in ipairs(m) do
		opt[p.name] = nil
	end

	local n = 1
	for o in pairs(opt) do
		list[n] = o
		n = n + 1
	end

	local dlg
	local but = iup.button{title = "Add", action = function() dlg:hide() end}
	local cancel = iup.button{title = "Cancel", action = function() list.value = -1 dlg:hide() end}
	dlg = iup.dialog{iup.vbox{
		iup.label{title = "Select a property to add:"},
		list, iup.hbox{but, cancel; gap = 10, alignment = "ACENTER"},
		gap = 12, nmargin = "15x15", alignment = "ACENTER"}}
	iup.Popup(dlg)

	if list.value ~= -1 then
		local str = list[list.value]
		local t = options[str]
		table.insert(m, {name = str, type = t, value = (t == 0) and 0 or ""})
		SaveNewProperty()
	end

	iup.Destroy(dlg)
end

return button
