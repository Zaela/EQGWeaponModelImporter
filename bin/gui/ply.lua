
local ply = {}

local tonumber = tonumber
local insert = table.insert
local ipairs = ipairs

function ply.Import(path)
	local f = assert(io.open(path, "r"))
	local str = f:read("*a")
	f:close()

	local vertex_count = tonumber(str:match("element vertex (%d+)"))
	local face_count = tonumber(str:match("element face (%d+)"))
	local _, n = str:find("end_header\n")
	n = n + 1

	--defaults
	local materials = {
		{name = "Custom", opaque = "Opaque_MaxCBSGE1.fx"}
	}

	--vertices are listed first
	local vertices = {}
	for a = 1, vertex_count do
		local line = str:match(".-\n", n)
		n = n + line:len()

		local x,y,z,i,j,k,u,v = line:match("(%-?[%d%.]+) (%-?[%d%.]+) (%-?[%d%.]+) (%-?[%d%.]+) (%-?[%d%.]+) (%-?[%d%.]+) (%-?[%d%.]+) (%-?[%d%.]+)")
		vertices[a] = {
			--position
			x = tonumber(x),
			y = tonumber(y),
			z = tonumber(z),
			--normal
			i = tonumber(i),
			j = tonumber(j),
			k = tonumber(k),
			--texture coords
			u = tonumber(u),
			v = tonumber(v)
		}
	end

	--then faces
	local triangles = {}
	for a = 1, face_count do
		local line = str:match(".-\n", n)
		n = n + line:len()
		local num = tonumber(line:match("%d+"))
		--the number of vertices in a ply face may be greater than 3; EQ only accepts exactly 3
		local a,b,c = line:match("%d+ (%d+) (%d+) (%d+)")
		a = tonumber(a)
		c = tonumber(c)
		insert(triangles, {
			[1] = a,
			[2] = tonumber(b),
			[3] = c,
			--defaults:
			group = 0,
			flag = 65536
		})
		if num > 3 then
			--so we have to break down larger polygons after the first triangle in a systematic way
			local _, x = line:find("%d+ %d+ %d+ %d+")
			line = line:sub(x + 1)
			--first, prev last, next
			local first = a
			local prev_last = c
			for d in line:gmatch("%d+") do
				local next = tonumber(d)
				insert(triangles, {
					[1] = first,
					[2] = prev_last,
					[3] = next,
					--defaults:
					group = 0,
					flag = 65536
				})
				prev_last = next
			end
		end
	end

	return {
		materials = materials,
		vertices = vertices,
		triangles = triangles,
		bones = {},
		bone_assignments = {},
	}
end

function ply.Export(data, path)
	local f = assert(io.open(path, "w+"))
	f:write("ply\n")
	f:write("format ascii 1.0\n")

	f:write("element vertex ".. #data.vertices, "\n")
	f:write("property double x\nproperty double y\nproperty double z\n")
	f:write("property double nx\nproperty double ny\nproperty double nz\n")
	f:write("property double s\nproperty double t\n")

	f:write("element face ".. #data.triangles, "\n")
	f:write("property list uchar uint vertex_indices\n")

	f:write("end_header\n")

	for _, v in ipairs(data.vertices) do
		f:write(v.x, " ", v.y, " ", v.z, " ", v.i, " ", v.j, " ", v.k, " ", v.u, " ", v.v, "\n")
	end

	for _, t in ipairs(data.triangles) do
		f:write("3 ", t[1], " ", t[2], " ", t[3], "\n")
	end

	f:close()
end

return ply
