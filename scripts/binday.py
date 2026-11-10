import datetime as dt
import click
import requests
from bs4 import BeautifulSoup
import re

def get_bins(loc:str):
    req = requests.get(f'https://onlineservices.glasgow.gov.uk/forms/refuseandrecyclingcalendar/CollectionsCalendar.aspx?UPRN={loc}')

    soup = BeautifulSoup(req.text, 'html.parser')

    bins = soup.find_all('li')
    
    regex = re.compile(r'((Mon|Tues|Wednes|Thurs|Fri|Satur|Sun)day\s\d+(th|st|nd|rd)\s\w+\s\d+|Tomorrow)')
    regex_bin = re.compile(r'(Blue|Green|Food)')
    
    due_tomorrow = False

    tomorrow = (dt.datetime.now() + dt.timedelta(days=1)).replace(hour=0,minute=0,second=0,microsecond=0)

    raw_html=f''
    for b in bins:
        out = b.get_text().strip()
        date = regex.search(out).group()
        colour = regex_bin.search(out).group()

        date_split = date.split()
        if date_split[0] == 'Tomorrow':
            due_tomorrow = True
            click.echo(f'{colour} bin @ {tomorrow.date()}')
            raw_html += f'<p>{colour} @ {tomorrow.date()}</p>'

    return due_tomorrow, raw_html

def transmit(key:str,address:str,sender:str,msg_html:str):
    
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
                'subject': f'Bin day tomorrow',
                'html_body': msg_html,
                }
            )

@click.command()
@click.argument('key')
@click.argument('address')
@click.argument('sender')
@click.argument('loc')
def binday(key:str,address:str,sender:str,loc:str):
    click.echo(f'Checking bins for {loc}')

    send_notif, html = get_bins(loc)

    if send_notif:
        click.echo('Bins tomorrow')
        transmit(key, address, sender, html)
    else:
        click.echo('No bins tomorrow :)')

if __name__ == '__main__':
    binday()
