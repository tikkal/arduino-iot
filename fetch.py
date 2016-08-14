#!/usr/bin/env python

import csv
from time import sleep
import StringIO
from subprocess import check_output

DELAY_SECS = 60
OUTFILE = "/var/www/html/tide/current.txt"
TIME_COL = 1
TEXT_COL = 3
ACCOUNTS = ["@NY1weather", "@nice_tips_bot"]
NUM_TWEETS = 1

def CleanText(text):
  end = text.find("http")
  if (text == "Text"):
    return ""
  if (end == -1):
    return text
  else:
    return text[0:end] + "\n" 

def Fetch(account):
  formatted = ""
  command = "/usr/local/bin/t timeline %s -n %d -c" % (account, NUM_TWEETS) 
  print "Fetching data with command: %s" % command
  twitter_csv = check_output(command, shell=True)
  if (len(twitter_csv) > 0):
    print "Received twitter csv data for %s." % account
    infile = StringIO.StringIO(twitter_csv)
    reader = csv.reader(infile, delimiter=',')
    formatted = ""
    for row in reader:
      formatted += CleanText(row[3])

  else:
    print "No twitter data received."
  return formatted

def Runner():
  formatted = ""
  for account in ACCOUNTS:
    formatted += Fetch(account)
    formatted += "\n"
  print(formatted)
  outfile = open(OUTFILE, 'w')
  outfile.write(formatted)
  outfile.close()
  print "Wrote data to %s." % OUTFILE

Runner()

