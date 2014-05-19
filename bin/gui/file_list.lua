
local eqg = require "luaeqg"
local ply = require "gui/ply"

local list = iup.list{visiblelines = 10, expand = "VERTICAL", visiblecolumns = 16,
	sort = "YES"}

local filter = iup.text{visiblecolumns = 15, value = ""}
file_filter = filter

local GatherFiles, FilterFileList

function filter:valuechanged_cb()
	local dir = open_dir
	if dir then
		GatherFiles(dir)
		FilterFileList()
	end
end

local ipairs = ipairs
local pairs = pairs
local pcall = pcall
local by_name

function GatherFiles(dir)
	for i, ent in ipairs(dir) do
		local name, ext = ent.name
		if name:find("%.mod$") or name:find("%.pts$") or name:find("%.prt$") then
			name, ext = ent.name:match("^(%w+)%.(%a%a%a)$")
			if name and ext then
				local bn = by_name[name]
				if not bn then
					ent.pos = i
					by_name[name] = {[ext] = ent, name = name}
				elseif not bn[ext] then
					ent.pos = i
					bn[ext] = ent
				end
			end
		end
	end
end

function UpdateFileList(path, silent)
	selection = nil
	eqg.CloseDirectory(open_dir)
	open_path = path
	local s, dir = pcall(eqg.LoadDirectory, path)
	if not s then
		error_popup(dir)
		return
	end
	open_dir = dir
	by_name = {}
	GatherFiles(dir)
	if not silent then
		FilterFileList()
	end
end

function FilterFileList(dir)
	list[1] = nil
	list.autoredraw = "NO"
	local f = filter.value
	local n = 1
	if f:len() > 0 then
		--make dot only match dot, not any and all characters
		f = f:gsub("%.", "%%%.")
		--make sure last char isn't a dangling % (will throw incomplete pattern error)
		if f:find("%%", -1) then
			f = f .. "%"
		end
		for name, tbl in pairs(by_name) do
			if name:find(f) and tbl.mod then
				list[n] = name
				n = n + 1
			end
		end
	else
		for name, tbl in pairs(by_name) do
			if tbl.mod then
				list[n] = name
				n = n + 1
			end
		end
	end
	list.autoredraw = "YES"
end

function RefreshSelection()
	local sel = selection
	local path = open_path
	if not sel or not path then return end
	UpdateFileList(path, true)
	selection = by_name[sel.name]
end

function list:action(str, pos, state)
	if state == 1 then
		local sel = by_name[str]
		if selection ~= sel then
			selection = sel
			local s, data = pcall(eqg.OpenEntry, sel.mod)
			if s then
				s, data = pcall(mod.Read, sel.mod)
				if s then
					model = data
					ClearDisplay()
					UpdateDisplay(data, str, sel)
					return
				end
			end
			error_popup(data)
		end
	end
end

function SaveDirEntry(entry, name)
	local dir = open_dir
	if not dir or not open_path then return end
	for i, ent in ipairs(dir) do
		if ent.name == name then
			dir[i] = entry
			s, err = pcall(eqg.WriteDirectory, open_path, dir)
			if not s then
				error_popup(err)
			end
			UpdateFileList(open_path, true)
			return
		end
	end
end

function Export()
	local dlg = iup.filedlg{title = "Export to...", dialogtype = "DIR"}
	iup.Popup(dlg)
	if dlg.status == "0" then
		local path = dlg.value
		local val = list.value
		if path and val then
			local str = list[list.value]
			local outpath = path .."\\".. str .. ".ply"
			local data = by_name[str].mod
			local s, err = pcall(eqg.OpenEntry, data)
			if s then
				s, err = pcall(mod.Read, data)
				if s then
					ply.Export(err, outpath)
					local msg = iup.messagedlg{title = "Export Status", value = "Export to ".. outpath .." complete."}
					iup.Popup(msg)
					iup.Destroy(msg)
					iup.Destroy(dlg)
					return
				end
			end
			error_popup(err)
		end
	end
	iup.Destroy(dlg)
end

function Import()
	local dlg = iup.filedlg{title = "Select file to import", dialogtype = "FILE",
		extfilter = "Stanford PLY (*.ply)|*.ply|"}
	iup.Popup(dlg)
	if dlg.status == "0" then
		local path = dlg.value
		local dir = open_dir
		if path and dir then
			local id = 1000
			local input = iup.text{visiblecolumns = 12, mask = iup.MASK_UINT}
			local getid
			local but = iup.button{title = "Done", action = function() id = tonumber(input.value) or 1000 getid:hide() end}
			getid = iup.dialog{iup.vbox{
				iup.label{title = "Please enter an ID number for the incoming weapon model:"},
				input, but, gap = 12, nmargin = "15x15", alignment = "ACENTER"};
				k_any = function(self, key) if key == iup.K_CR then but:action() end end}
			iup.Popup(getid)
			iup.Destroy(getid)
			local name = "it".. id
			local pos = by_name[name]
			if pos then
				pos = pos.mod.pos
				local warn = iup.messagedlg{title = "Overwrite?",
					value = "A model with ID ".. id .." already exists in this archive. Overwrite it?",
					buttons = "YESNO", dialogtype = "WARNING"}
				iup.Popup(warn)
				local yes = (warn.buttonresponse == "1")
				iup.Destroy(warn)
				if not yes then return end
			else
				pos = #dir + 1
			end
			local data = ply.Import(path)
			name = name .. ".mod"
			local s, err = pcall(mod.Write, data, name, eqg.CalcCRC(name))
			if s then
				dir[pos] = err
				s, err = pcall(eqg.WriteDirectory, open_path, dir)
				if s then
					local msg = iup.messagedlg{title = "Import Status", value = "Import of ".. name .." complete."}
					iup.Popup(msg)
					UpdateFileList(open_path)
					iup.Destroy(msg)
					iup.Destroy(dlg)
					return
				end
			end
			error_popup(err)
		end
	end
	iup.Destroy(dlg)
end
--[=[
function Delete()
	local dlg = iup.messagedlg{buttondefault = "2", title = "Confirm Deletion",
		buttons = "OKCANCEL", dialogtype = "WARNING", value = "Are you sure you want to delete this file?"}
	iup.Popup(dlg)
	if dlg.buttonresponse == "2" then return end
	local dir = open_dir
	local path = open_path
	local val = list.value
	if dir and path and val then
		table.remove(dir, by_name[list[val]].pos)
		local s, err = pcall(eqg.WriteDirectory, path, dir)
		if s then
			UpdateFileList(path)
			iup.Destroy(dlg)
			return
		end
		error_popup(err)
	end
	iup.Destroy(dlg)
end
]=]
function list:button_cb(button, pressed, x, y)
	if button == iup.BUTTON3 and pressed == 0 then
		local has = selection and "YES" or "NO"
		local mx, my = iup.GetGlobal("CURSORPOS"):match("(%d+)x(%d+)")
		local menu = iup.menu{
			iup.item{title = "Export Selected Model to .ply", action = Export, active = has},
			iup.item{title = "Import Model from .ply", action = Import},
			--iup.separator{},
			--iup.item{title = "Delete Selected File", action = Delete, active = has},
		}
		iup.Popup(menu, mx, my)
		iup.Destroy(menu)
	end
end

return iup.vbox{iup.hbox{iup.label{title = "Filter"}, filter; alignment = "ACENTER", gap = 5}, list;
	alignment = "ACENTER", gap = 5}
