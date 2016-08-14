#!/usr/bin/env python

import csv
from time import sleep
import StringIO
from subprocess import check_output

DELAY_SECS = 60
OUTFILE = "/var/www/html/tide/current.txt"
TIME_COL = 1
TEXT_COL = 3

def CleanText(text):
  end = text.find("http")
  if (text == "Text"):
    return ""
  if (end == -1):
    return text
  else:
    return text[0:end] + "\n" 

while True:
  account = "@NY1weather"
  command = "t timeline %s -n 3 -c" % account
  print "Fetching data with command: %s" % command
  twitter_csv = check_output(command, shell=True)
  if (len(twitter_csv) > 0):
    print "Received twitter csv data:"
    infile = StringIO.StringIO(twitter_csv)
    reader = csv.reader(infile, delimiter=',')
    formatted = ""
    for row in reader:
      formatted += CleanText(row[3])
    print formatted 
    outfile = open(OUTFILE, 'w')
    outfile.write(formatted)
    outfile.close()
    print "Wrote data to %s." % OUTFILE
  else:
    print "No twitter data received."

  print "Sleeping for %d seconds." % DELAY_SECS
  sleep(DELAY_SECS) 

