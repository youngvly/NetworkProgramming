#try : python test.py

from xml.dom import minidom
def readxml() :
	#parse an xmlfile by name
	mydoc = minidom.parse('output.xml')
	
	record = mydoc.getElementsByTagName('record')
	data = record[0].getElementsByTagName('data')

	#one spcifir item's data
	#print('\nItem #2 data :')
	#print(items[1].firstChild.data)
	#print(items[1].childNodes[0].data)

	#all items data
	print('\nAll item data :')
	for i in data:
		print(i.getElementsByTagName('num')[0].childNodes[0].data),
		print(i.getElementsByTagName('username')[0].childNodes[0].data),
		print(i.getElementsByTagName('message')[0].childNodes[0].data),
		print(i.getElementsByTagName('time')[0].childNodes[0].data)	
readxml()	
