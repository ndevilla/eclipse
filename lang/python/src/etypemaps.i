%include typemaps.i

%typemap(python,in) char ** { 
/* Check if is a list */ 
    if (PyList_Check($source)) { 
	int size = PyList_Size($source); 
	int i = 0; 
	$target = (char **) malloc((size)*sizeof(char *)); 
	for (i = 0; i < size; i++) { 
	    PyObject *o = PyList_GetItem($source,i); 
	    if (PyString_Check(o)) 
		$target[i] = PyString_AsString(PyList_GetItem($source,i)); 
	    else { 
		PyErr_SetString(PyExc_TypeError,"list must contain strings"); 
	        free($target); 
		return NULL; 
	    } 
        } 
    } 
    else { 
	PyErr_SetString(PyExc_TypeError,"not a list"); 
	return NULL; 
    } 
} 

%typemap(python,freearg) char ** { 
    free((char *) $source); 
}

// Map a returned size_t to a Python integer
%typemap(python,out) size_t {
    $target = Py_BuildValue("l",(long)*$source);
}

// Map a returned pixelvalue to a Python float
%typemap(python,out) pixelvalue {
    $target = Py_BuildValue("f",(double)$source);
}

%typemap(python, out) image_stats *{
    if ($source){
    	$target = Py_BuildValue("ddddddddlllll",
				(double) $source->min_pix,
				(double) $source->max_pix,
			        (double) $source->avg_pix,
				(double) $source->median_pix,
				(double) $source->stdev,
			        (double) $source->energy,
			        (double) $source->flux,
			        (double) $source->absflux,
				(long)   $source->min_x,
				(long)   $source->min_y,
				(long)   $source->max_x,
				(long)   $source->max_y,
				(long)   $source->npix);
    }
    else{
	$target = Py_BuildValue("");
    }
}

//Map a list of ints to an array of ints
%typemap(python, in) int * {
    	if ($source == Py_None){
		$target = NULL;
    	}
    	else{
		if (PyList_Check($source)) { 
			int size = PyList_Size($source); 
			int i = 0; 
			$target = (int *) malloc((size)*sizeof(int)); 
			for (i = 0; i < size; i++) { 
	    			PyObject *o = PyList_GetItem($source,i); 
	    			if (PyInt_Check(o)) {
					$target[i] = (int)PyInt_AsLong(o); 
				}
				else { 
					PyErr_SetString(PyExc_TypeError,"list must contain strings"); 
			       		free($target); 
					return NULL; 
				} 
    			}
		}
    		else { 
			PyErr_SetString(PyExc_TypeError,"not a list"); 
			return NULL; 
    		} 
	}
}

%typemap(python,freearg) int * { 
    	if ($source != NULL){
		free( $source); 
	}
}

//map a list of floats to an array of doubles
%typemap(python, in) double * {
	if ($source == Py_None){
		$target = NULL;
	}
	else {
		if (PyList_Check($source)) { 
			int size = PyList_Size($source); 
			int i = 0; 
			$target = (double *) malloc((size)*sizeof(double)); 
			for (i = 0; i < size; i++) { 
	    			PyObject *o = PyList_GetItem($source,i); 
	    			if (PyFloat_Check(o)) {
					$target[i] = PyFloat_AsDouble(o); 
				}
	    			else { 
					PyErr_SetString(PyExc_TypeError,"list must contain floats"); 
	        			free($target); 
					return NULL; 
	                	} 
               		}
    		}
	    	else { 
			PyErr_SetString(PyExc_TypeError,"not a list");
			return NULL; 
        	} 
	}
}

%typemap(python,freearg) double * { 
	if ($source != NULL){
	    free( $source); 
	}
}

//map a list of floats to an array of pixelvalues
%typemap(python, in) pixelvalue * {
	if ($source == Py_None){
		$target = NULL;
	}
    	else {
		if (PyList_Check($source)) { 
			int size = PyList_Size($source); 
			int i = 0; 
			$target = (float *) malloc((size)*sizeof(pixelvalue)); 
			for (i = 0; i < size; i++) { 
	    			PyObject *o = PyList_GetItem($source,i); 
	    			if (PyFloat_Check(o)) { 
				$target[i] = (pixelvalue)PyFloat_AsDouble(o); 
				}
	    			else { 
					PyErr_SetString(PyExc_TypeError,"list must contain floats"); 
	        			free($target); 
					return NULL; 
	    			} 
        		} 
    		} 
		else { 
			PyErr_SetString(PyExc_TypeError,"not a list");
			return NULL; 
    		} 
	}
} 


%typemap(python,freearg) pixelvalue * { 
	if ($source != NULL){
	    free( $source); 
	}
}

%typemap(python, out) char * {
    if ($source == NULL){
       Py_INCREF(Py_None);
       $target = Py_None;
    } else {
       $target = PyString_FromString($source);
    }
}
