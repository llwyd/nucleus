import datetime as dt
import click
import requests
from bs4 import BeautifulSoup
import re

@click.command()
@click.argument('loc')
def binday(loc:str):
    click.echo(loc)

    req = requests.get(f'https://onlineservices.glasgow.gov.uk/forms/refuseandrecyclingcalendar/CollectionsCalendar.aspx?UPRN={loc}')

    soup = BeautifulSoup(req.text, 'html.parser')

    bins = soup.find_all('li')
    
    regex = re.compile(r'(Mon|Tues|Wednes|Thurs|Fri|Satur|Sun)day\s\d+(th|st|nd|rd)\s\w+\s\d+')
    regex_bin = re.compile(r'(Blue|Green|Food)')
    
    due_tomorrow = False

    tomorrow = (dt.datetime.now() + dt.timedelta(days=1)).replace(hour=0,minute=0,second=0,microsecond=0)
    click.echo(tomorrow)
    
    for b in bins:
        out = b.get_text().strip()
        date = regex.search(out).group()
        colour = regex_bin.search(out).group()
        click.echo(f'Bin: {colour} @ {date}')

        date_split = date.split()
        timestamp = dt.datetime.strptime(f'{date_split[0]} {date_split[1][:-2]} {date_split[2]} {date_split[3]}','%A %d %B %Y')
        click.echo(timestamp)
        if timestamp.date() == tomorrow.date():
            due_tomorrow = True


if __name__ == '__main__':
    binday()
