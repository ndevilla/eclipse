-- collapse.lua
-- eclipse script to replace the 'collapse' command

-- Change collapse parameters here

params =  {
	dir="vertical",
	median=1,
	reject = {
		min=0,
		max=0
	}
}

name_i = args[2]
if args[3] then
	name_o = args[3]
else
	name_o = "collapsed.fits"
end

in = load(name_i)
if not in then
	print("cannot load "..name_i)
	return -1
end

col = collapse(in, params)
if not col then
	print("error collapsing "..name_i)
	return -1
end

save(col, name_o)
print("done.")
