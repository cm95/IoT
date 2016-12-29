import paho.mqtt.client as paho
import os
import time
import MySQLdb as mdb
from twilio.rest import TwilioRestClient 
from datetime import datetime, timedelta
import math 

#create an mqtt client
host = "co657-mqtt.kent.ac.uk"
port = 1883
mypid = os.getpid()
client_uniq = "cm610"+str(mypid)
mqttc = paho.Client(client_uniq)

#try to connect to sever - will keep trying 
def connectMQTT():
	isConnected = False 
	
	while not isConnected: 
		try:
			mqttc.connect(host, port, 60)
			isConnected = True 
			print "connected"
		except: 
			print "cannot connect"
			pass 
	

#runs until error, checking new data from serial in log then posts to topic
#checks connection to mqtt,database will try to reconnect if not connnected 
while True:
	connectMQTT()

	with open('log.txt') as fp:
		for line in fp:
		
			#this will send a text if the bin needs emptied 
			if int(line) < 10:
				ACCOUNT_SID = "ACa7b9ce40a44b0fe00d0ae6bdeb2a3d8e" 
				AUTH_TOKEN = "97d19eec312bc494bd79eb1d76624520" 
				 
				client = TwilioRestClient(ACCOUNT_SID, AUTH_TOKEN) 
				 
				client.messages.create(
					to="+447969513095", 
					from_="+441687292010	", 
					body="testing", 
				)
				
		
			#connect to database and populate 	
			isSqlConnected = False 
	
			while not isSqlConnected: 
				try:
					conn = mdb.connect('dragon.kent.ac.uk', 'cm610', '*PASSWORD*', 'cm610');
					isSqlConnected = True 
					print "connected"
				except: 
					print "cannot connect"
					pass 				
			
			print "Opened database successfully"
			cur = conn.cursor()

			#make state 	
			if int(line) < 10:
				state = "'full'"
				sql = "SELECT * FROM sensorData WHERE state = 'empty' ORDER BY date DESC LIMIT 1" 
				cur.execute(sql)
				results = cur.fetchall()
				for row in results:
					date = row[2]
					timen = row[3]					  
				dateold = str(date) +" "+str(timen) 
				now = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
				datetimeFormat = '%Y-%m-%d %H:%M:%S'    

				timedelta = datetime.strptime(now, datetimeFormat) - datetime.strptime(dateold,datetimeFormat)

				day = str(math.floor(((timedelta).days) ))
				hour =  str(math.floor(((timedelta).seconds) / 3600))
				
			elif int(line) > 89:
				state = "'empty'"
				sql = "SELECT * FROM sensorData WHERE state = 'full' ORDER BY date DESC LIMIT 1" 
				cur.execute(sql)
				results = cur.fetchall()
				for row in results:
					date = row[2]
					timen = row[3]					  
				dateold = str(date) +" "+str(timen) 
				now = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
				datetimeFormat = '%Y-%m-%d %H:%M:%S'    

				timedelta = datetime.strptime(now, datetimeFormat) - datetime.strptime(dateold,datetimeFormat)

				day = str(math.floor(((timedelta).days) ))
				hour =  str(math.floor(((timedelta).seconds) / 3600))
			else:
				state = "'in use'"
				
			if int(line) < 10 or int(line) >89:
				cur.execute("INSERT INTO sensorData(cm,date,time,state,timetakendays,timetakenhours)VALUES ("+line+",CURDATE(),CURTIME(),"+state+","+day+","+hour+")");
			else: 
				cur.execute("INSERT INTO sensorData(cm,date,time,state)VALUES ("+line+",CURDATE(),CURTIME(),"+state+")");

			conn.commit()
			conn.close()
			print "Data submitted successfully"
			
		
			mqttc.publish("users/cm610/mysensor/ultrasonic",  line)
		
	fp.close()		
	fl = open('log.txt',"w+")
	for line in fl: 
		fl.write("\n")
	fl.close()	
	time.sleep(5)