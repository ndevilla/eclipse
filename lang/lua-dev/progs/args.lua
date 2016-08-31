-- Demonstration of command-line argument parsing

if args then
	for i,v in args do
		print("arg["..i.."] = ["..v.."]")
	end
else
	print("received no argument table")
end
