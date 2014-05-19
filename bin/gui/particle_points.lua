
local eqg = require "luaeqg"

local list = iup.list{visiblelines = 5, expand = "VERTICAL", visiblecolumns = 16}

local pcall = pcall
local ipairs = ipairs
local pairs = pairs
local edit_button, attach_list

local data, bones
function UpdateParticlePoints(pts, mod)
	data = pts
	list[1] = nil
	attach_list[2] = nil
	if not mod then return end
	local n = 2
	bones = {}
	for i, bone in ipairs(mod.bones) do
		attach_list[n] = bone.name
		bones[i] = bone.name
		n = n + 1
	end
	if not pts then return end
	list.autoredraw = "NO"
	for i, p in ipairs(pts) do
		list[i] = p.particle_name
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

attach_list = iup.list{visiblecolumns = 10, dropdown = "YES", "ATTACH_TO_ORIGIN",
	action = Edited, k_any = EnterKey}

local field = {
	name = iup.text{visiblecolumns = 12, readonly = "YES"},
	--attach = iup.text{visiblecolumns = 12, action = Edited, k_any = EnterKey},
	trans_x = iup.text{visiblecolumns = 12, mask = iup.MASK_FLOAT, action = Edited, k_any = EnterKey},
	trans_y = iup.text{visiblecolumns = 12, mask = iup.MASK_FLOAT, action = Edited, k_any = EnterKey},
	trans_z = iup.text{visiblecolumns = 12, mask = iup.MASK_FLOAT, action = Edited, k_any = EnterKey},
	rot_x = iup.text{visiblecolumns = 12, mask = iup.MASK_FLOAT, action = Edited, k_any = EnterKey},
	rot_y = iup.text{visiblecolumns = 12, mask = iup.MASK_FLOAT, action = Edited, k_any = EnterKey},
	rot_z = iup.text{visiblecolumns = 12, mask = iup.MASK_FLOAT, action = Edited, k_any = EnterKey},
	scale_x = iup.text{visiblecolumns = 12, mask = iup.MASK_FLOAT, action = Edited, k_any = EnterKey},
	scale_y = iup.text{visiblecolumns = 12, mask = iup.MASK_FLOAT, action = Edited, k_any = EnterKey},
	scale_z = iup.text{visiblecolumns = 12, mask = iup.MASK_FLOAT, action = Edited, k_any = EnterKey},
}

local grid = iup.gridbox{
	iup.label{title = "Identifier"}, field.name,
	iup.label{title = "Attach To"}, attach_list,--field.attach,
	iup.label{title = "Translation X"}, field.trans_x,
	iup.label{title = "Translation Y"}, field.trans_y,
	iup.label{title = "Translation Z"}, field.trans_z,
	iup.label{title = "Rotation X"}, field.rot_x,
	iup.label{title = "Rotation Y"}, field.rot_y,
	iup.label{title = "Rotation Z"}, field.rot_z,
	iup.label{title = "Scale X"}, field.scale_x,
	iup.label{title = "Scale Y"}, field.scale_y,
	iup.label{title = "Scale Z"}, field.scale_z,
	numdiv = 2, orientation = "HORIZONTAL", homogeneouslin = "YES",
	gapcol = 10, gaplin = 8, alignmentlin = "ACENTER", sizelin = 2
}

function list:action(str, pos, state)
	if state == 1 then
		local d = data[pos]
		if not d then return end
		point_selection = d
		field.name.value = d.particle_name
		--field.attach.value = d.attach_name
		field.trans_x.value = d.translation.x
		field.trans_y.value = d.translation.y
		field.trans_z.value = d.translation.z
		field.rot_x.value = d.rotation.x
		field.rot_y.value = d.rotation.y
		field.rot_z.value = d.rotation.z
		field.scale_x.value = d.scale.x
		field.scale_y.value = d.scale.y
		field.scale_z.value = d.scale.z

		local name = d.attach_name
		if name == "ATTACH_TO_ORIGIN" then
			attach_list.value = 1
		else
			for i, n in ipairs(bones) do
				if name == n then
					attach_list.value = i + 1
					break
				end
			end
		end

		local sel = selection
		if sel.prt then
			local s, data = pcall(eqg.OpenEntry, sel.prt)
			if s then
				s, data = pcall(prt.Read, sel.prt)
				if s then
					ClearParticleEntries()
					UpdateParticleEntries(data, str)
					return
				end
			end
			error_popup(data)
		else
			ClearParticleEntries()
			UpdateParticleEntries()
		end
	end
end

function ClearPointFields()
	for _, f in pairs(field) do
		f.value = ""
	end
	edit_button.active = "NO"
	point_selection = nil
	attach_list.value = 0
end

local function Save()
	local sel = selection
	local dir = open_dir
	local path = open_path
	if not sel or not data or not dir or not path then return end

	local name = sel.name .. ".pts"
	local s, d = pcall(pts.Write, data, name, eqg.CalcCRC(name))
	if s then
		local pos = sel.pts and sel.pts.pos or (#dir + 1)
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

local add_button = iup.button{title = "Add Point", padding = "10x0"}

function add_button:action()
	local name
	local input = iup.text{visiblecolumns = 12, nc = 63}
	local getname
	local but = iup.button{title = "Done", action = function() name = tostring(input.value) getname:hide() end}
	getname = iup.dialog{iup.vbox{
		iup.label{title = "Please enter a name to identify the new point:"},
		input, but, gap = 12, nmargin = "15x15", alignment = "ACENTER"};
		k_any = function(self, key) if key == iup.K_CR then but:action() end end}
	iup.Popup(getname)
	iup.Destroy(getname)

	if not name or name:len() < 1 then return end
	local point = {
		particle_name = name,
		attach_name = "ATTACH_TO_ORIGIN",
		translation = {x = 0, y = 0, z = 0},
		rotation = {x = 0, y = 0, z = 0},
		scale = {x = 1, y = 1, z = 1},
	}

	if not data then
		data = {}
	end
	data[#data + 1] = point

	Save()
	UpdateParticlePoints(data)
	ClearPointFields()
end

edit_button = iup.button{title = "Commit Changes", padding = "10x0"}

local tonumber = tonumber

function edit_button:action()
	local d = point_selection
	if not d then return end

	local v = tonumber(attach_list.value)
	d.attach_name = (v < 2) and "ATTACH_TO_ORIGIN" or bones[v - 1]
	--d.attach_name = field.attach.value
	d.translation.x = tonumber(field.trans_x.value)
	d.translation.y = tonumber(field.trans_y.value)
	d.translation.z = tonumber(field.trans_z.value)
	d.rotation.x = tonumber(field.rot_x.value)
	d.rotation.y = tonumber(field.rot_y.value)
	d.rotation.z = tonumber(field.rot_z.value)
	d.scale.x = tonumber(field.scale_x.value)
	d.scale.y = tonumber(field.scale_y.value)
	d.scale.z = tonumber(field.scale_z.value)

	Save()
	edit_button.active = "NO"
	local sel = selection
	local s, update = pcall(pts.Read, sel.pts)
	if s then
		data = update
	end
end

function RefreshPointSelection()
	local d = point_selection
	if not d or not data then return end

	for i, point in ipairs(data) do
		if point == d then
			list:action(d.particle_name, i, 1)
			return
		end
	end
end

return iup.hbox{
	iup.vbox{iup.label{title = "Emission Points"}, list, add_button;
		gap = 10, alignment = "ACENTER"},
	iup.vbox{grid, edit_button; gap = 52, alignment = "ACENTER"};
	gap = 10, alignment = "ACENTER"}
