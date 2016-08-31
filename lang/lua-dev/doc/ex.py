import string
import glob

list = glob.glob("../src/*.c")
for i in list:
	f = open(i)
	l = f.readline()
	while l:
		if string.find(l, "/**")!=-1:
			print("-"*60)
			l = f.readline()
			while string.find(l, "*/")==-1:
				print l[:-1]
				l = f.readline()
			print("-"*60)
			print ""
		else:
			l = f.readline()
	f.close()

