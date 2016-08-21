#!/usr/bin/env python

from twitter import AddTwitterOptions 
from twitter import TwitterFetcher
from wu import AddWuOptions 
from wu import WeatherFetcher

from optparse import OptionParser

MASH_OUTFILE = "/var/www/html/tide/current.txt"
WEATHER_OUTFILE = "/var/www/html/tide/weather.txt"

def WriteData(content, filename):
  outfile = open(filename, 'w')
  outfile.write(content)
  outfile.close()
  print("Wrote to {}.".format(filename))


if __name__ == '__main__':
  parser = OptionParser()
  AddWuOptions(parser)
  AddTwitterOptions(parser)
  (options, args) = parser.parse_args()
  weather_fetcher = WeatherFetcher(options)
  weather_json = weather_fetcher.fetch()
  temp_f = '{}\n'.format(weather_json['temp_f'])
  print(temp_f)
  WriteData(temp_f, WEATHER_OUTFILE)
  twitter_fetcher = TwitterFetcher(options)

  mashed = '{}\n{}\n\n'.format(
        weather_json['weather'],
        weather_json['temperature_string'])
  mashed += twitter_fetcher.fetch()
  print(mashed)
  WriteData(mashed, MASH_OUTFILE); 
