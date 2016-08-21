#!/usr/bin/env python 

import datetime
import json
import urllib2
import sys
from optparse import OptionParser

class WeatherFetcher:
  def __init__(self, options):
    self.api_key = 'your_api_key'
    self.loc_id = options.loc_id
    self.verb = options.verb

  def read_from_server(self):
    query_prefix = 'http://api.wunderground.com/api/' + self.api_key
    verb_prefix = query_prefix + '/' + self.verb + '/q/'
    request_url = verb_prefix + self.loc_id + '.json'
    return json.load(urllib2.urlopen(request_url))

  def get_json_response(self):
    json_response = self.read_from_server()
    return json_response

  def fetch(self):
    json = self.get_json_response()
    cur = json['current_observation']
    return cur

def AddWuOptions(parser):
  parser.add_option('-v', '--verb',
                  action='store',
                  default='conditions',
                  dest='verb',
                  help='Specify verb',
                  type='string')
  parser.add_option('-l', '--loc_id',
                  action='store',
                  default='us/ny/new-york',
                  dest='loc_id',
                  help='Specify location',
                  type='string')

if __name__ == '__main__':
  parser = OptionParser()
  AddWuOptions(parser)
  (options, args) = parser.parse_args()
  weather_fetcher = WeatherFetcher(options)
  # json_response = weather_reader.get_json_response()
  # print json.dumps(json_response, sort_keys=True, indent=2, separators=(',', ': '))
  formatted = weather_fetcher.fetch()
  print formatted
