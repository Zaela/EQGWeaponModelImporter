
local list = iup.list{visiblelines = 5, expand = "VERTICAL", visiblecolumns = 6}

local ipairs = ipairs

local data
local cur
function UpdateParticleEntries(prt, name)
	data = prt
	list[1] = nil
	if not prt then return end
	list.autoredraw = "NO"
	local n = 1
	cur = {}
	for _, p in ipairs(prt) do
		if p.particle_name == name then
			list[n] = tostring(n)
			cur[n] = p
			n = n + 1
		end
	end
	list.autoredraw = "YES"
end

local field = {
	id = iup.text{visiblecolumns = 12, mask = iup.MASK_UINT},
	duration = iup.text{visiblecolumns = 12, mask = iup.MASK_UINT},
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
		t = iup.text{visiblecolumns = 12}
	else
		t = iup.text{visiblecolumns = 12, mask = iup.MASK_UINT}
	end
	field.unknown[i] = t
	iup.Append(grid, iup.label{title = "Unknown".. i})
	iup.Append(grid, t)
end

function list:action(str, pos, state)
	if state == 1 then
		local p = cur[pos]
		if not p then return end
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
end

local button = iup.button{title = "Add Particle", padding = "10x0"}

local edit_button = iup.button{title = "Commit Changes", padding = "10x0"}

return iup.hbox{
	iup.vbox{iup.label{title = "Attached Particles"}, list, button;
		gap = 10, alignment = "ACENTER"},
	iup.vbox{grid, edit_button; gap = 75, alignment = "ACENTER"};
	gap = 10, alignment = "ACENTER"}
