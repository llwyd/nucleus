import matplotlib.pyplot as plt
from matplotlib.figure import Figure
import numpy as np
import sqlalchemy as db
import pandas as pd
import datetime as dt

engine = db.create_engine('sqlite:///app.db')
conn = engine.connect()
metadata = db.MetaData()

environment_data = db.Table('environment_data', metadata, autoload_with=engine)

#query = environment_data.select().where(environment_data.columns.device_id == "bedroom")

query = environment_data.select()
query_output = conn.execute(query)

df = pd.DataFrame(query_output.fetchall())

df['temperature'] = df['temperature'].astype(float)
df['humidity'] = df['humidity'].astype(float)

nodes = df.device_id.unique()

test_date = "2025-10-24"

todays_date            = dt.datetime.now().strftime('%Y-%m-%d')
yesterdays_date        = (dt.datetime.now() - dt.timedelta(days=1)).strftime('%Y-%m-%d') 
yesterdays_time        = (dt.datetime.now() - dt.timedelta(days=1))

for node in nodes:
    data = df.copy()
    data = data[data['device_id'] == node]
    data['datetime'] = pd.to_datetime(data['datestamp'] + " " + data['timestamp'])
    data = data[data.datetime > yesterdays_time]
    #d_time = [dt.datetime.strptime(date,'%Y-%m-%d %H:%M') for date in data['datetime']]
    plt.plot(data['datetime'], np.float32(data['temperature']))

plt.legend(nodes)
plt.xticks(rotation=45)
plt.show()

