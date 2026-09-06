import numpy as np
import sqlalchemy as db
import pandas as pd
import datetime as dt
import click
import requests

def get_todays_events(db_path:str):
    
    engine = db.create_engine(db_path)
    conn = engine.connect()
    metadata = db.MetaData()

    event_data = db.Table('event_data', metadata, autoload_with=engine)

    query = event_data.select()
    query_output = conn.execute(query)

    df = pd.DataFrame(query_output.fetchall())

    num_events = len(df)
    todays_date            = dt.datetime.now().strftime('%Y-%m-%d')
    yesterdays_time        = (dt.datetime.now() - dt.timedelta(minutes=5)) 
 
    num_events = 0
    raw_html = f'<table style ="margin-left:auto;margin-right:auto;"><tr><th><b>ID</b></th><th><b>Date</b></th><th><b>Time</b></th><th><b>Device</b></th><th><b>Event</b></th></tr>'
    for idx, reading in df.iterrows():
        row_dt = pd.to_datetime(reading.datestamp + " " + reading.timestamp)
        if row_dt > yesterdays_time:
            row_html = f'<tr><td>{reading.id}</td><td>{reading.datestamp}</td><td>{reading.timestamp}</td><td>{reading.device_id}</td><td>{reading.event}</td></tr>'
            raw_html += row_html
            num_events += 1

    raw_html += f'</table>'
    return num_events, raw_html

@click.command()
@click.argument('key')
@click.argument('address')
@click.argument('sender')
@click.argument('path')
def digest(key:str, address:str, sender:str, path):
    db_path = f'sqlite:///{path}'
    click.echo(db_path)
 
    num_events,raw_html = get_todays_events(db_path)
    
    click.echo(f'num events: {num_events}')
    url = 'https://api.smtp2go.com/v3/email/send'
    headers = {
        'Content-Type': 'application/json',
        'X-Smtp2go-Api-Key': f'{key}',
        'accept': 'application/json',
            }

'''
    r = requests.post(
            url,
            headers=headers,
            json={
                'sender': f'{sender}',
                'to': [f'{address}'],
                'subject': f'{num_events} event(s) detected',
                'html_body': raw_html,
                }
            )
'''

if __name__ == '__main__':
    digest()
