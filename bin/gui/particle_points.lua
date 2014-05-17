
local eqg = require "luaeqg"

local list = iup.list{visiblelines = 5, expand = "VERTICAL", visiblecolumns = 16}

local ipairs = ipairs
local pairs = pairs

local data
function UpdateParticlePoints(pts)
	data = pts
	list[1] = nil
	if not pts then return end
	list.autoredraw = "NO"
	for i, p in ipairs(pts) do
		list[i] = p.particle_name
	end
	list.autoredraw = "YES"
end

local field = {
	name = iup.text{visiblecolumns = 12},
	attach = iup.text{visiblecolumns = 12},
	trans_x = iup.text{visiblecolumns = 12, mask = iup.MASK_FLOAT},
	trans_y = iup.text{visiblecolumns = 12, mask = iup.MASK_FLOAT},
	trans_z = iup.text{visiblecolumns = 12, mask = iup.MASK_FLOAT},
	rot_x = iup.text{visiblecolumns = 12, mask = iup.MASK_FLOAT},
	rot_y = iup.text{visiblecolumns = 12, mask = iup.MASK_FLOAT},
	rot_z = iup.text{visiblecolumns = 12, mask = iup.MASK_FLOAT},
	scale_x = iup.text{visiblecolumns = 12, mask = iup.MASK_FLOAT},
	scale_y = iup.text{visiblecolumns = 12, mask = iup.MASK_FLOAT},
	scale_z = iup.text{visiblecolumns = 12, mask = iup.MASK_FLOAT},
}

local grid = iup.gridbox{
	iup.label{title = "Identifier"}, field.name,
	iup.label{title = "Attach To"}, field.attach,
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
		field.name.value = d.particle_name
		field.attach.value = d.attach_name
		field.trans_x.value = d.translation.x
		field.trans_y.value = d.translation.y
		field.trans_z.value = d.translation.z
		field.rot_x.value = d.rotation.x
		field.rot_y.value = d.rotation.y
		field.rot_z.value = d.rotation.z
		field.scale_x.value = d.scale.x
		field.scale_y.value = d.scale.y
		field.scale_z.value = d.scale.z

		local sel = selection
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
	end
end

function ClearPointFields()
	for _, f in pairs(field) do
		f.value = ""
	end
end

local button = iup.button{title = "Add Point", padding = "10x0"}

local edit_button = iup.button{title = "Commit Changes", padding = "10x0"}

return iup.hbox{
	iup.vbox{iup.label{title = "Emission Points"}, list, button;
		gap = 10, alignment = "ACENTER"},
	iup.vbox{grid, edit_button; gap = 52, alignment = "ACENTER"};
	gap = 10, alignment = "ACENTER"}
