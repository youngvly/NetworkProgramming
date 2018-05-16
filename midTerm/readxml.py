#compile : python test.py

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
	print('\n 		<<<SHOW ALL CHATTING MESSAGE>>>')
	print('{:-<80}'.format('-'))
	print(' {:3}'.format('num')+' | ' +'{:41}'.format(' message')+' | ' +'{:26}'.format('sent time'))
	print('{:-<80}'.format('-'))
	for i in data:
		print(' {:3}'.format(i.getElementsByTagName('num')[0].childNodes[0].data) + ' | '),
		#because message include username.
		#print(i.getElementsByTagName('username')[0].childNodes[0].data),
		
		#to remove \n from message
		message = i.getElementsByTagName('message')[0].childNodes[0].data.splitlines()
		#to meet table format, if message is longer than 40words, use next line
		if (len(message[0])<=40) :
			print('{:40}'.format(message[0]) + ' | '),
		else :
			print(message[0])
			print(' {:3}'+' | ' +'{:40}'+' | '),
		#to remove '(space)' end of time message
		time = i.getElementsByTagName('time')[0].childNodes[0].data[:-1] + '\0'
		print('{:26}'.format(time))	
	print('{:-<80}'.format('-'))
readxml()
