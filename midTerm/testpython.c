#include <stdio.h>
#include <python2.7/Python.h>

int main() {
//	char filename[] = "readxml.py";	
//	FILE * fp = fopen(filename,"r");
	Py_Initialize();

	//File "<string>",line1,in<module> error 
	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.path");
	PyRun_SimpleString("sys.path.append('/home/young/Desktop/network_Programming/NetworkProgramming/midTerm')");

	PyRun_SimpleString("import readxml");
	PyRun_SimpleString("print readxml.readxml()");
//	PyRun_SimpleFile(fp,filename);
	Py_Finalize();
//	fclose(fp);
	return 0;
}
