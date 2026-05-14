import matplotlib.pyplot as plt
from matplotlib.figure import Figure
import matplotlib.dates as mdates
import numpy as np
import sqlalchemy as db
import pandas as pd
import datetime as dt
import click
import requests
import base64
from io import BytesIO

def get_todays_data(db_path:str):
    t_fig = Figure(figsize=(8,6))
    h_fig = Figure(figsize=(8,6))
    t_ax = t_fig.subplots()
    h_ax = h_fig.subplots()
    
    engine = db.create_engine(db_path)
    conn = engine.connect()
    metadata = db.MetaData()

    environment_data = db.Table('environment_data', metadata, autoload_with=engine)

    query = environment_data.select()
    query_output = conn.execute(query)

    df = pd.DataFrame(query_output.fetchall())

    df['temperature'] = df['temperature'].astype(float)
    df['humidity'] = df['humidity'].astype(float)

    nodes = df.device_id.unique()
    todays_date            = dt.datetime.now().strftime('%Y-%m-%d')
    yesterdays_date        = (dt.datetime.now() - dt.timedelta(days=1)).strftime('%Y-%m-%d') 
    yesterdays_time        = (dt.datetime.now() - dt.timedelta(days=1)) 
    
    for node in nodes:
        data = df.copy()
        data = data[data['device_id'] == node]
        data['datetime'] = pd.to_datetime(data['datestamp'] + " " + data['timestamp'])
        data = data[data.datetime > yesterdays_time]
        if not data.empty:
            t_ax.plot(data['datetime'], np.float32(data['temperature']),label=node)
            h_ax.plot(data['datetime'], np.float32(data['humidity']),label=node)
    
    t_ax.set_title('Temperature',color='white')
    t_ax.set_xlabel('Time', color='white')
    t_ax.set_ylabel('Degrees (C)', color='white')
    t_ax.tick_params('x', labelrotation=45, colors='white')
    t_ax.tick_params('y', colors='white')
    t_ax.spines[:].set_color('white')
    t_ax.legend()

    date_format = mdates.DateFormatter('%H:%M')
    t_ax.xaxis.set_major_formatter(date_format) 
    
    h_ax.set_title('Humidity',color='white')
    h_ax.set_xlabel('Time',color='white')
    h_ax.set_ylabel('Relative Humidity',color='white')
    h_ax.tick_params('x', labelrotation=45, colors='white')
    h_ax.tick_params('y', colors='white')
    h_ax.spines[:].set_color('white')
    h_ax.legend()
    
    h_ax.xaxis.set_major_formatter(date_format) 
   
    t_buf = BytesIO()
    t_fig.savefig(t_buf, format='png',transparent=True)
    
    h_buf = BytesIO()
    h_fig.savefig(h_buf, format='png',transparent=True)
    return base64.b64encode(t_buf.getbuffer()).decode("ascii"), base64.b64encode(h_buf.getbuffer()).decode("ascii")

@click.command()
@click.argument('key')
@click.argument('address')
@click.argument('sender')
@click.argument('path')
def digest(key:str, address:str, sender:str, path):
    db_path = f'sqlite:///{path}'
    click.echo(db_path)
    graph, h_graph = get_todays_data(db_path)
  
    url = 'https://api.smtp2go.com/v3/email/send'
    headers = {
        'Content-Type': 'application/json',
        'X-Smtp2go-Api-Key': f'{key}',
        'accept': 'application/json',
            }
    r = requests.post(
            url,
            headers=headers,
            json={
                'sender': f'{sender}',
                'to': [f'{address}'],
                'subject': 'Digest',
                'html_body': f'<img src=\"data:image/png;base64, {graph}\"/><img src=\"data:image/png;base64, {h_graph}\"/>',
                }
            )

if __name__ == '__main__':
    digest()
