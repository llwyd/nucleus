import matplotlib.pyplot as plt
from matplotlib.figure import Figure
import numpy as np
import sqlalchemy as db
import pandas as pd
import datetime as dt
import click
import requests

@click.command()
@click.argument('key')
@click.argument('address')
@click.argument('sender')
@click.argument('path')
def digest(key:str, address:str, sender:str, path):
    db_path = f'sqlite:///{path}'
    click.echo(db_path)

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
                  'text_body': 'hello'}
            )

if __name__ == '__main__':
    digest()
