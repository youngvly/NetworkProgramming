#include <stdio.h>
#include <python2.7/Python.h>

int main(int argc, char*argv[]) {
//	char filename[] = "readxml.py";	
//	FILE * fp = fopen(filename,"r");
	Py_Initialize();

	//File "<string>",line1,in<module> error 
	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.path");
	PyRun_SimpleString("sys.path.append('/home/young/Desktop/network_Programming/NetworkProgramming/midTerm')");

	PySys_SetArgv(argc,argv);
	PyObject *name,*time,*msg,*pdic,*pmodule,*pfunc,*pargs,*funcname;
	name = PyString_FromString("kim");
	time = PyString_FromString("time");
	msg = PyString_FromString("msg");
	funcname = PyString_FromString("writexml");
	pmodule = PyImport_Import(funcname);
	pdic = PyModule_GetDict(pmodule);
	pfunc= PyDict_GetItem(pdic,funcname);
	pargs = PyTuple_New(3);
	PyTuple_SetItem(pargs,0,name);
	PyTuple_SetItem(pargs,1,time);
	PyTuple_SetItem(pargs,2,msg);
	PyObject_CallObject(pfunc,pargs);
	
	Py_DECREF(pdic);
	Py_DECREF(name);
	Py_DECREF(time);
	Py_DECREF(msg);
	Py_DECREF(pargs);

	PyRun_SimpleString("import readxml");
	PyRun_SimpleString("print readxml.readxml()");

//	PyRun_SimpleFile(fp,filename);
	Py_Finalize();
//	fclose(fp);
	return 0;
}
