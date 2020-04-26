from flask import Flask, redirect, render_template, request, url_for
from flask_sqlalchemy import SQLAlchemy
from flask_basicauth import BasicAuth
from flask_caching import Cache
from sqlalchemy import or_
import datetime as dt
import dash
import dash_core_components as dcc
import dash_html_components as html
import plotly.graph_objs as go
import plotly
import devices as dv
from dash.dependencies import Input, Output
import numpy as np
import os
basedir = os.path.abspath(os.path.dirname(__file__))

external_stylesheets = ['https://codepen.io/chriddyp/pen/bWLwgP.css']

app = dash.Dash(__name__, external_stylesheets=external_stylesheets)

app.config.suppress_callback_exceptions = True

app.title='Home Assistant'
app.server.config["DEBUG"] = True

cache_config = {
    "DEBUG": True,         
    "CACHE_TYPE": "simple", 
    "CACHE_DEFAULT_TIMEOUT": 300
}

app.server.config.from_mapping(cache_config)
cache = Cache(app.server)

SQLALCHEMY_DATABASE_URI = os.environ.get('DATABASE_URL') or 'sqlite:///' + os.path.join(basedir, 'app.db')
app.server.config["SQLALCHEMY_DATABASE_URI"] = SQLALCHEMY_DATABASE_URI
app.server.config["SQLALCHEMY_POOL_RECYCLE"] = 299
app.server.config["SQLALCHEMY_TRACK_MODIFICATIONS"] = False

db = SQLAlchemy(app.server)

# Database model
class Readings(db.Model):
    __tablename__ = "readings"
    id              = db.Column(db.Integer, primary_key=True)       #id for each record in the database
    deviceID        = db.Column(db.String(64))                      # id of the unit
    datestamp       = db.Column(db.String(32))                     	# datestamp
    timestamp       = db.Column(db.String(32))                     	# timestamp
    temperature     = db.Column(db.String(16))                      # temperature
    humidity        = db.Column(db.String(16))                      # humidity

#   Entry point for the website
app.layout = html.Div(children=[

    dcc.Location(id='url', refresh=False),
    html.Div(id='page_content')
])


#   URL Handler
@app.callback(Output('page_content', 'children'),[Input('url', 'pathname')])
def display_page(pathname):
    return index_page

#   Static Index Page
index_page = html.Div([
    html.H1(children='Home Assistant'),
   	html.Div(id = 'last-update'), 
    html.Div(id = 'server-uptime'),
	dcc.Interval(   id='interval-component',
                    interval = 1000 * 60 * 5,
                    n_intervals = 0
                ),
    dcc.Graph(id='static-temp-graph')
])

# last update text
@app.callback(Output('last-update', 'children'),[Input('interval-component','n_intervals')])
def last_update(n):
	last_entry = Readings.query.order_by(Readings.id.desc()).first()
	return [ html.Span("Last Update: " + str( last_entry.datestamp ) +" at " +str( last_entry.timestamp ) ) ]

# server uptime
@app.callback(Output('server-uptime', 'children'),[Input('interval-component','n_intervals')])
def update_uptime(n):
    if( cache.get("server_uptime") is not None):
        server_uptime = cache.get("server_uptime")
        return [ html.Span("Server Uptime: " + str(server_uptime))]	
    else:
        return [ html.Span("Server Uptime: ????")]	

#   Temperature graph
@app.callback(Output('static-temp-graph', 'figure'),[Input('interval-component','n_intervals')])
def static_temp_graph(value):
    todays_date 		= dt.datetime.now().strftime('%Y-%m-%d')
    yesterdays_date 	= (dt.datetime.now() - dt.timedelta(days=1)).strftime('%Y-%m-%d') 

    yesterdays_time 	= (dt.datetime.now() - dt.timedelta(days=1)) 
    
    todays_data = db.session.query( Readings ).filter( or_(Readings.datestamp == todays_date, Readings.datestamp == yesterdays_date) ).all()
    data = {
            'x':[],
            'y':[],
            't':[],
            'z':[],}

    for d in todays_data:
        d_time = dt.datetime.strptime(d.datestamp + " " + d.timestamp,'%Y-%m-%d %H:%M')
        if(d_time >= yesterdays_time):
            if(d.deviceID==dv.devices[0]['deviceID']):
                data['x'].append(d.timestamp)
                data['y'].append(d.temperature)
            elif(d.deviceID==dv.devices[1]['deviceID']):
                data['z'].append(d.temperature)
                data['t'].append(d.timestamp)
    figure={
        'data': [
            {'x': data['x'], 'y': data['y'], 'type': 'scatter', 'name': 'Inside'},
            {'x': data['t'], 'y': data['z'], 'type': 'scatter', 'name': 'Outside'}
        ],
        'layout': {
            'title': '24 Hour Temperature Monitoring',
            'xaxis': {
                'title': 'Time'
            },
            'yaxis': {
                'title': 'Temperature (Degrees)'
            }
        },

    }
    return figure;

@app.server.route('/raw', methods = ['GET', 'POST'])
def raw_data():
    if request.method == 'POST':
        datestamp = dt.datetime.now().strftime('%Y-%m-%d')
        timestamp = dt.datetime.now().strftime('%H:%M')
        raw = request.get_json(force=True)
        print(raw)
        reading = Readings(	deviceID 	= raw['device_id'],
				datestamp 				= datestamp,
				timestamp 				= timestamp,
				temperature 			= raw['temperature'],
				humidity 				= raw['humidity'])
        db.session.add(reading)
        db.session.commit()
    return 'ta pal\n'

@app.server.route('/stats', methods = ['GET', 'POST'])
def receive_stats():
    if request.method == 'POST':
        raw = request.get_json(force = True)
        server_uptime = str( raw['Days']) + " Day(s), " + str(raw['Hours']) + " hour(s) and " + str(raw['Minutes']) + " minute(s)."
        cache.set("server_uptime",server_uptime);
    return 'ta pal\n'

@app.server.route('/dump', methods = ['GET', 'POST'])
def dump_data():
	return render_template("raw_data.html", readings = Readings.query.all())

if __name__ == '__main__':
    app.run_server(host='0.0.0.0')
