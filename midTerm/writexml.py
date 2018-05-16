import xml.etree.ElementTree as ET
from xml.dom import minidom

def makeRecordFile() :
	ET.register_namespace("","http://www.w3.org/2000/svg")
	root = ET.Element("record")
	tree = ET.ElementTree(root)
	tree.write("output.xml")	

def writexml(msguser,msgtime,msgdata) :
	msgnum = 0

	ET.register_namespace("","http://www.w3.org/2000/svg")

	tree = ET.parse('output.xml')
	root = tree.getroot()
	lastdata = len(root)-1
	if (msgnum !=0) : 
		msgnum =  int(root[lastdata][0].text)
	
	newdata = ET.SubElement(root,"data")
	
	ET.SubElement(newdata,"num").text=str(int(msgnum)+1)
	ET.SubElement(newdata,"username").text = msguser
	ET.SubElement(newdata,"time").text = msgtime
	ET.SubElement(newdata,"message").text = msgdata

	#ET.dump(root)
	tree.write('output.xml')

