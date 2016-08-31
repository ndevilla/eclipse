
REF = reference.txt

doc:
	rm -f $(REF)
	touch $(REF)
	echo "eclipse/Lua reference manual" >> $(REF)
	echo "Creation date:" `date` >> $(REF)
	python ex.py >> $(REF)
