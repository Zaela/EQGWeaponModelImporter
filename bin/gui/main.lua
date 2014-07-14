
local lfs = require "lfs"
local eqg = require "luaeqg"

function assert(result, msg)
	if result then return result end
	local err = iup.messagedlg{buttons = "OK", dialogtype = "ERROR", title = "Error", value = msg}
	iup.Popup(err)
	iup.Close()
end

function error_popup(msg)
	local err = iup.messagedlg{buttons = "OK", dialogtype = "ERROR", title = "Error", value = msg}
	iup.Popup(err)
	iup.Destroy(err)
end

local dir_list = require "gui/dir_list"
local file_list = require "gui/file_list"
local display = require "gui/display"

local function SetSearchFolder()
	local dlg = iup.filedlg{title = "Select EQG Search Folder", dialogtype = "DIR", directory = lfs.currentdir()}
	iup.Popup(dlg)
	if dlg.status == "0" then
		local path = dlg.value
		if path then
			search_path = path
			UpdateDirList(path)
			local v = settings and settings.viewer
			local f = assert(io.open("gui/settings.lua", "w+"))
			f:write("\nsettings = {\n\tfolder = \"", (path:gsub("\\", "\\\\")), "\",\n")
			f:write("\tviewer = {\n\t\twidth = ", v and v.width or 500, ",\n\t\theight = ", v and v.height or 500, ",\n")
			f:write("\t}\n}\n")
			f:close()
		end
	end
end

local function LoadSettings()
	local set = loadfile("gui/settings.lua")
	if set then
		set()
		if settings and settings.folder then
			search_path = settings.folder
			UpdateDirList(search_path)
			return
		end
	end
	SetSearchFolder()
end

LoadSettings()
LoadSettings = nil

local menu = iup.menu{
	iup.submenu{
		title = "&File";
		iup.menu{
			iup.item{title = "Set EQG Search Folder", action = SetSearchFolder},
			iup.separator{},
			iup.item{title = "&Quit", action = function() return iup.CLOSE end},
		},
	},
	iup.submenu{
		title = "Viewer";
		iup.menu{
			iup.item{title = "Start Viewer", action = viewer.Open},
			iup.separator{},
			iup.item{title = "Stop Viewer", action = viewer.Close},
		},
	},
}

local window = assert(iup.dialog{iup.hbox{dir_list, file_list, display; nmargin = "10x10", gap = 10};
	title = "EQG Weapon Model Importer v1.1", menu = menu, size = "610x250"})

function window:k_any(key)
	if key == iup.K_ESC then
		return iup.CLOSE
	end
end

window:show()
iup.MainLoop()

eqg.CloseDirectory(open_dir)

iup.Close()
