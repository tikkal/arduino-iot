#!/usr/bin/env python

import csv
import StringIO
from optparse import OptionParser
from subprocess import check_output

class TwitterFetcher:
  def __init__(self, options):
    self.account = options.twitter_account
    self.num_tweets = options.num_tweets
    self.time_col = 1
    self.text_col = 3

  def clean_text(self, text):
    end = text.find("http")
    if (text == "Text"):
      return ""
    if (end == -1):
      return text
    else:
      return text[0:end] + "\n"

  def fetch_one_account(self, account):
    formatted = ""
    command = "/usr/local/bin/t timeline %s -n %d -c" % (account,
        self.num_tweets)
    twitter_csv = check_output(command, shell=True)
    if (len(twitter_csv) > 0):
      infile = StringIO.StringIO(twitter_csv)
      reader = csv.reader(infile, delimiter=',')
      formatted = ""
      for row in reader:
        formatted += self.clean_text(row[3])
    return formatted

  def fetch(self):
    formatted = self.fetch_one_account(self.account)
    formatted += "\n"
    return formatted

def AddTwitterOptions(parser):
  parser.add_option('--twitter_account', action = 'store',
                    default = '@WernerTwertzog', dest = 'twitter_account',
                    help = 'Twitter account to fetch from', type = 'string')
  parser.add_option('--num_tweets', action = 'store', default = 1,
                    dest = 'num_tweets', help = 'Number of tweets to fetch',
                    type = 'int')

if __name__ == '__main__':
  parser = OptionParser()
  AddTwitterOptions(parser)
  (options, args) = parser.parse_args()
  twitter_fetcher = TwitterFetcher(options)
  print twitter_fetcher.fetch()
