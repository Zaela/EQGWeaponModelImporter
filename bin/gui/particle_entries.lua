
local eqg = require "luaeqg"

local list = iup.list{visiblelines = 5, expand = "VERTICAL", visiblecolumns = 6}

local ipairs = ipairs
local edit_button

local data
local cur
function UpdateParticleEntries(prt, name)
	data = prt
	list[1] = nil
	cur = {}
	if not prt then return end
	list.autoredraw = "NO"
	local n = 1
	for _, p in ipairs(prt) do
		if p.particle_name == name then
			list[n] = tostring(n)
			cur[n] = p
			n = n + 1
		end
	end
	list.autoredraw = "YES"
end

local function Edited()
	edit_button.active = "YES"
end

local function EnterKey(self, key)
	if key == iup.K_CR then
		edit_button:action()
	end
end

local field = {
	id = iup.text{visiblecolumns = 12, mask = iup.MASK_UINT, action = Edited, k_any = EnterKey},
	duration = iup.text{visiblecolumns = 12, mask = iup.MASK_UINT, action = Edited, k_any = EnterKey},
	unknown = {},
}

local grid = iup.gridbox{
	iup.label{title = "Particle ID"}, field.id,
	iup.label{title = "Duration?"}, field.duration,
	numdiv = 2, orientation = "HORIZONTAL", homogeneouslin = "YES",
	gapcol = 10, gaplin = 8, alignmentlin = "ACENTER", sizelin = 0
}

for i = 1, 8 do
	local t
	if i == 7 then
		t = iup.text{visiblecolumns = 12, action = Edited, k_any = EnterKey}
	else
		t = iup.text{visiblecolumns = 12, mask = iup.MASK_UINT, action = Edited, k_any = EnterKey}
	end
	field.unknown[i] = t
	iup.Append(grid, iup.label{title = "Unknown".. i})
	iup.Append(grid, t)
end

local particle_selection
function list:action(str, pos, state)
	if state == 1 then
		local p = cur[pos]
		if not p then return end
		particle_selection = p
		field.id.value = p.particle_id
		field.duration.value = p.duration
		local u = p.unknown
		for i, v in ipairs(u) do
			if i == 7 then
				field.unknown[7].value = string.format("0x%0.8X", v)
			else
				field.unknown[i].value = v
			end
		end
	end
end

function ClearParticleEntries()
	list[1] = nil
	field.id.value = ""
	field.duration.value = ""
	for _, f in ipairs(field.unknown) do
		f.value = ""
	end
	edit_button.active = "NO"
	particle_selection = nil
end

local function Save()
	local sel = selection
	local dir = open_dir
	local path = open_path
	if not sel or not data or not dir or not path then return end

	local name = sel.name .. ".prt"
	local s, d = pcall(prt.Write, data, name, eqg.CalcCRC(name))
	if s then
		local pos = sel.prt and sel.prt.pos or (#dir + 1)
		dir[pos] = d
		d.pos = pos
		s, d = pcall(eqg.WriteDirectory, path, dir)
		if s then
			RefreshSelection()
			return
		end
	end
	error_popup(d)
end

local add_button = iup.button{title = "Add Particle", padding = "10x0"}

function add_button:action()
	--add to end of both 'data' and 'cur'
	local point = point_selection
	if not point or not cur then return end

	local p = {
		particle_id = 0,
		particle_name = point.particle_name,
		duration = 5000,
		unknown = {},
	}
	for i = 1, 8 do
		if i == 7 then
			p.unknown[7] = 0xFFFFFFFF
		else
			p.unknown[i] = 0
		end
	end

	if not data then
		data = {}
	end
	data[#data + 1] = p
	cur[#cur + 1] = p

	Save()
	RefreshPointSelection()
end

edit_button = iup.button{title = "Commit Changes", padding = "10x0"}

local tonumber = tonumber

function edit_button:action()
	local p = particle_selection
	if not p then return end

	p.particle_id = tonumber(field.id.value)
	p.duration = tonumber(field.duration.value)
	for i = 1, 8 do
		p.unknown[i] = tonumber(field.unknown[i].value)
	end

	Save()
	edit_button.active = "NO"
	local sel = selection
	local s, update = pcall(prt.Read, sel.prt)
	if s then
		UpdateParticleEntries(update, point_selection.particle_name)
	end
end

return iup.hbox{
	iup.vbox{iup.label{title = "Attached Particles"}, list, add_button;
		gap = 10, alignment = "ACENTER"},
	iup.vbox{grid, edit_button; gap = 75, alignment = "ACENTER"};
	gap = 10, alignment = "ACENTER"}
