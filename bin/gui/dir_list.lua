
local lfs = require "lfs"
local eqg = require "luaeqg"

local list = iup.list{visiblelines = 10, expand = "VERTICAL", visiblecolumns = 14, sort = "YES"}

local filter = iup.text{visiblecolumns = 13, value = ""}

function filter:valuechanged_cb()
	local path = search_path
	if path then
		UpdateDirList(path)
	end
end

function UpdateDirList(path)
	list[1] = nil
	list.autoredraw = "NO"
	local i = 1
	local f = filter.value
	if f:len() > 0 then
		--make dot only match dot, not any and all characters
		f = f:gsub("%.", "%%%.")
		--make sure last char isn't a dangling % (will throw incomplete pattern error)
		if f:find("%%", -1) then
			f = f .. "%"
		end
		for str in lfs.dir(path) do
			if str:find(f) and str:find("%.eqg", -4) then
				list[i] = str
				i = i + 1
			end
		end
	else
		for str in lfs.dir(path) do
			if str:find("%.eqg", -4) then
				list[i] = str
				i = i + 1
			end
		end
	end
	list.autoredraw = "YES"
end

function list:action(str, pos, state)
	if state == 1 then
		local path = search_path .."\\".. str
		ClearDisplay()
		UpdateFileList(path)
	end
end

local function NewArchive()
	local path = search_path
	if not path then return end
	local name
	local input = iup.text{visiblecolumns = 12, mask = "/w+"}
	local dlg
	local button = iup.button{title = "Done",
		action = function()
			name = (input.value ~= "") and input.value or "unnamed"
			dlg:hide()
		end}
	dlg = iup.dialog{iup.vbox{
		iup.label{title = "Enter a name for your new EQG archive:"},
		input, button; gap = 12, nmargin = "15x15", alignment = "ACENTER"}}
	iup.Popup(dlg)
	iup.Destroy(dlg)
	if not name then return end
	name = path .."\\".. name .. ".eqg"
	local s, err = pcall(eqg.WriteDirectory, name, {})
	if s then
		UpdateDirList(path)
	else
		error_popup(err)
	end
end

function list:button_cb(button, pressed, x, y)
	if button == iup.BUTTON3 and pressed == 0 then
		local mx, my = iup.GetGlobal("CURSORPOS"):match("(%d+)x(%d+)")
		local menu = iup.menu{
			iup.item{title = "New EQG Archive", action = NewArchive},
		}
		iup.Popup(menu, mx, my)
		iup.Destroy(menu)
	end
end

return iup.vbox{iup.hbox{iup.label{title = "Filter"}, filter; alignment = "ACENTER", gap = 5}, list;
	alignment = "ACENTER", gap = 5}
