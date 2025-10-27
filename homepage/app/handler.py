from app import app, db, mqtt
from app.models import EnvironmentData
from app.models import EventData
import json
import datetime as dt

def database_update(name, temperature, humidity):
    datestamp = dt.datetime.now().strftime('%Y-%m-%d')
    timestamp = dt.datetime.now().strftime('%H:%M')
    reading = EnvironmentData(device_id = name, datestamp = datestamp, timestamp = timestamp, temperature = temperature, humidity = humidity)
    with app.app_context():
        db.session.add(reading)
        db.session.commit()

def database_event_update(device, event):
    datestamp = dt.datetime.now().strftime('%Y-%m-%d')
    timestamp = dt.datetime.now().strftime('%H:%M')
    reading = EventData(device_id = device, datestamp = datestamp, timestamp = timestamp, event = event)
    with app.app_context():
        db.session.add(reading)
        db.session.commit()

@mqtt.on_connect()
def handle_connect(client,userdata,flags,rc):
    print("MQTT Connected")
    mqtt.subscribe('home/evnt/#')
    mqtt.subscribe('home/summary/#')
    mqtt.subscribe('home/digest/#')

@mqtt.on_topic('home/evnt/#')
def handle_event_data(client,userdata,message):
    full_topic = message.topic
    node = full_topic.replace('/',' ').split()[-1]
    event = full_topic.replace('/',' ').split()[-2]
    database_event_update(node,event)
    #print(data)

@mqtt.on_topic('home/summary/#')
def handle_summary_data(client,userdata,message):
    full_topic = message.topic
    node = full_topic.replace('/',' ').split()[-1]
    data_str = message.payload.decode()
    data = json.loads(data_str)
    database_update(node, data['temperature'], data['humidity'])
    #print(data)

@mqtt.on_topic('home/digest/#')
def handle_digest_data(client,userdata,message):
    full_topic = message.topic
    node = full_topic.replace('/',' ').split()[-1]
    data_str = message.payload.decode()
    data = json.loads(data_str)
    database_update(node, data['t'], data['h'])
    #print(data)

