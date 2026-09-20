from app import app, db, mqtt
from app.models import EnvironmentData
from app.models import EventData
import json
import datetime as dt
import base64 as b64 

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
    mqtt.subscribe('home/digest/#')

@mqtt.on_topic('home/evnt/#')
def handle_event_data(client,userdata,message):
    full_topic = message.topic
    node = full_topic.replace('/',' ').split()[-1]
    event = full_topic.replace('/',' ').split()[-2]
    database_event_update(node,event)

@mqtt.on_topic('home/digest/#')
def handle_digest_data(client,userdata,message):
    full_topic = message.topic
    node = full_topic.replace('/',' ').split()[-1]
    data_b64 = message.payload.decode()

    # Is it valid b64?
    try:
        data_str = b64.urlsafe_b64decode(bytes(data_b64,encoding='utf-8')).decode('utf-8')
        # Is it valid json?
        try:
            data = json.loads(data_str)
            database_update(node, data['t'], data['h'])
        except:
            pass
    except:
        pass

