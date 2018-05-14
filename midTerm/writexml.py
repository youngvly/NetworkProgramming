import xml.etree.ElementTree as ET

def writexml(msguser,msgtime,msgdata) :
	
	ET.register_namespace("","http://www.w3.org/2000/svg")
#	ET.register_namespace("","http://www.sitemaps.org/schemas/sitemap/0.9")
	tree = ET.parse('output.xml')
	root = tree.getroot()
	
	newdata = ET.SubElement(root,"data")
	
	ET.SubElement(newdata,"username").text = msguser
	ET.SubElement(newdata,"time").text = msgtime
	ET.SubElement(newdata,"message").text = msgdata

	ET.dump(root)
	tree.write('output.xml')

writexml("Kim","2018-05-14 18:33","~Hello")
