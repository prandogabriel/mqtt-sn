from AWSIoTPythonSDK.MQTTLib import AWSIoTMQTTClient
from random import randint
import math
import sys
import logging
import time
import json
import datetime
import argparse
import boto3
# import parser

dynamodb = boto3.resource('dynamodb', region_name='us-east-1')                    # connection to DynamoDB and access
dynamoTable = dynamodb.Table('tutorial')                                     # to the table EnvironmentalStation that will store the data provided
jsonP = '';                                                                         # by the two simulated stations

def data_store(client, userdata, msg):                                              # the function stores the recieved data in the DynamoDB table
    payload = str(msg.payload)[2:-1]
    jsonP = json.loads(payload)
    dynamoTable.put_item(Item=jsonP)


def send_data(myClient, data, topic):                                               # the function publishes the recieved data
    messageJson = json.dumps(data)
    myClient.publish(topic, messageJson, 1)
    print("########## DATA RECIEVED ##########")
    print("Published on topic: %s\nData provided by: %s\nRecieved data:\n%s\n" % (topic, clientId, messageJson))
    print("I'm sending the data to DynamoDB... \n \n")

def random_values():                                                                 # the function provides in a simple way random environmental values...
    temperature = str(randint(-50,50))
    humidity = str(randint(0, 100))
    wind_direction = str(randint(0, 360))
    wind_intensity = str(randint(0, 100))
    rain_height = str(randint(0, 50))
    datatime = str(datetime.datetime.now())[:19]                                     #... and take notice about the time of the detection, that is important for data storing in the table

    return temperature, humidity, wind_direction, wind_intensity, rain_height, datatime

def awsconnection(useWebsocket = False,                                          # the function sets the connection to AWS
    clientId = "",                                                               # client id
    thingName = "tutorial",                                               # thing name (on AWS)
    host = "a27i3bls8pki37-ats.iot.us-east-1.amazonaws.com",                                                      # your AWS endpoint
    caPath = "./keys/AmazonRootCA1.pem",                                             # rootCA certificate (folder's path)
    certPath = "./keys/certificate.pem.crt",                       # client certificate (folder's path)
    keyPath = "./keys/private.pem.key"                             # private key (folder's path)
    ):

    port = 8883 if not useWebsocket else 443
    useWebsocket = useWebsocket
    clientId = clientId
    host = host
    port = port
    rootCaPath = caPath
    privateKeyPath = keyPath
    certificatePath = certPath

    # Logger settings
    # more information at https://docs.python.org/3/library/logging.html
    logger = logging.getLogger("AWSIoTPythonSDK.core")
    logger.setLevel(logging.NOTSET)                                                      # NOTSET causes all messages to be processed when the logger is the root logger
    streamHandler = logging.StreamHandler()                                              # sends logging output to streams
    formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    streamHandler.setFormatter(formatter)
    logger.addHandler(streamHandler)

    # AWSIoTMQTTClient initialization
    myClient = None
    if useWebsocket:
        myClient = AWSIoTMQTTClient(clientId, useWebsocket=True)
        myClient.configureEndpoint(host, port)
        myClient.configureCredentials(rootCaPath)
    else:
        myClient = AWSIoTMQTTClient(clientId)
        myClient.configureEndpoint(host, port)
        myClient.configureCredentials(rootCaPath, privateKeyPath, certificatePath)

    # AWSIoTMQTTClient connection configuration
    myClient.configureAutoReconnectBackoffTime(1, 32, 20)
    myClient.configureOfflinePublishQueueing(-1)      # param: if set to 0, the queue is disabled. If set to -1, the queue size is set to be infinite.
    myClient.configureDrainingFrequency(2)            # Draining: 2 Hz
    myClient.configureConnectDisconnectTimeout(10)    # 10 sec
    myClient.configureMQTTOperationTimeout(5)         # 5 sec

    return myClient


# Arguments passing: in this case the only argument that we need is the clientId, that is supposed to be station1 or station2
# parser = argparse.ArgumentParser()
# parser.add_argument('--clientid', type=str)
# args = parser.parse_args()
clientId = "stationId"
topic = "tutorial"


myClient =  awsconnection(clientId=clientId)
myClient.connect()
myClient.subscribe(topic, 1, data_store)

# Every 5 seconds, random data are generated, sent and published
while True:

    temperature, humidity, wind_direction, wind_intensity, rain_height, datatime = random_values()

    data = {"id":clientId, "datetime":datatime, "Temperature":temperature, "Humidity":humidity,
           "WindDirection":wind_direction, "WindIntensity":wind_intensity, "RainHeight":rain_height}

    send_data(myClient, data, topic)

    time.sleep(5)

myClient.disconnect()
