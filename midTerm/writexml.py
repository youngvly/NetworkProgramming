import xml.etree.ElementTree as ET
from xml.dom import minidom

def writexml(msguser,msgtime,msgdata) :
	
	ET.register_namespace("","http://www.w3.org/2000/svg")

	tree = ET.parse('output.xml')
	root = tree.getroot()
	data = ET.SubElement(root,"data")
	lastdata = len(root)-2
	msgnum =  int(root[lastdata][0].text)
	
	newdata = ET.SubElement(root,"data")
	
	ET.SubElement(newdata,"num").text=str(int(msgnum)+1)
	ET.SubElement(newdata,"username").text = msguser
	ET.SubElement(newdata,"time").text = msgtime
	ET.SubElement(newdata,"message").text = msgdata

	ET.dump(root)
	tree.write('output.xml')

writexml("Kim","2018-05-14 18:33","~Hello")
