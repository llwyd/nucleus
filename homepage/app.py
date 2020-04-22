from flask import Flask, redirect, render_template, request, url_for
from flask_sqlalchemy import SQLAlchemy
from flask_basicauth import BasicAuth
import datetime as dt
import dash
import dash_core_components as dcc
import dash_html_components as html
import plotly.graph_objs as go
import plotly
from dash.dependencies import Input, Output
import numpy as np
import os
basedir = os.path.abspath(os.path.dirname(__file__))

external_stylesheets = ['https://codepen.io/chriddyp/pen/bWLwgP.css']

app = dash.Dash(__name__, external_stylesheets=external_stylesheets)

app.config.suppress_callback_exceptions = True

app.title='home Assistant v 0.0.1'
app.server.config["DEBUG"] = True


SQLALCHEMY_DATABASE_URI = os.environ.get('DATABASE_URL') or 'sqlite:///' + os.path.join(basedir, 'app.db')
app.server.config["SQLALCHEMY_DATABASE_URI"] = SQLALCHEMY_DATABASE_URI
app.server.config["SQLALCHEMY_POOL_RECYCLE"] = 299
app.server.config["SQLALCHEMY_TRACK_MODIFICATIONS"] = False

db = SQLAlchemy(app.server)

# Database model
class Readings(db.Model):
    __tablename__ = "readings"
    id              = db.Column(db.Integer, primary_key=True)       #id for each record in the database
    deviceID        = db.Column(db.String(64))                      #id of the unit
    datestamp       = db.Column(db.String(128))                     #datestamp
    temperature     = db.Column(db.String(16))                      #temperature
    humidity        = db.Column(db.String(16))                      #humidity

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
    html.H1(children='Home Assistant Version 0.1'),
    html.H3(children='Temperature Data'),
    html.H3(children='Server Info'),
    dcc.Interval(   id='interval-component',
                    interval = 1000 * 60 * 5,
                    n_intervals = 0
                ),
    #dcc.Dropdown(   id='date-dropdown',
    #                style={'height':'35px','width':'200px'},
    #                options=[
    #                            {'label':'test','value':'1234'},
    #                        ],
    #                clearable=False,
    #                placeholder="Select a data"
    #                #value = 0
    #                ),
    dcc.Graph(id='static-temp-graph')
])

#   Temperature graph
@app.callback(Output('static-temp-graph', 'figure'),[Input('interval-component','n_intervals')])
def static_temp_graph(value):
    data={}
    print("Hello")
    data['y']=np.random.rand(1000);
    data['x']=np.linspace(0,999,1000);
    figure={
        'data': [
            {'x': data['x'], 'y': data['y'], 'type': 'scatter', 'name': 'Noise'}
        ],
        'layout': {
            'title': 'Noise',
            'xaxis': {
                'title': 'Time (Seconds)'
            },
            'yaxis': {
                'title': 'Amplitude'
            }
        },

    }
    return figure;

@app.server.route('/raw', methods = ['GET', 'POST'])
def raw_data():
    if request.method == 'POST':
        timestamp = dt.datetime.now()
        raw = request.get_json(force=True)
        print(raw)
        reading = Readings(	deviceID = raw['device_id'],
				datestamp = timestamp,
				temperature = raw['inside_temp'],
				humidity = '0')
        db.session.add(reading)
        db.session.commit()
    return 'ta pal\n'

if __name__ == '__main__':
    app.run_server(host='0.0.0.0')
