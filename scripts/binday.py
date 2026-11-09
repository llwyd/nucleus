import datetime as dt
import click
import requests
from bs4 import BeautifulSoup

@click.command()
@click.argument('loc')
def binday(loc:str):
    click.echo(loc)

    req = requests.get(f'https://onlineservices.glasgow.gov.uk/forms/refuseandrecyclingcalendar/CollectionsCalendar.aspx?UPRN={loc}')

    soup = BeautifulSoup(req.text, 'html.parser')

    bins = soup.find_all('li')

    for b in bins:
        click.echo(b.get_text().lstrip().rstrip())


if __name__ == '__main__':
    binday()
