#!/usr/bin/env python

import sqlite3
import argparse
from datetime import datetime, timedelta

DEFAULT_DB_FILE = "ttnmapping.sqlite"
DEFAULT_OUTFILE = "geodata.js"

class gen_geo:
    """connect to a ttnmapper sqlite db to generate geojson lines from each row

    Attributes:
        filename: the filename to hold the database
    """

    def __init__(self, filename=DEFAULT_DB_FILE):
        self.sqlite_file = filename
        self.conn = sqlite3.connect(self.sqlite_file)
        self.conn.row_factory = sqlite3.Row
        self.cur = self.conn.cursor()

    def fetch_all(self):
        with self.conn:
            self.cur.execute("select * from messages;")
            this = self.cur.fetchall()
            return this

    def fetch_last_hours(self, last_hours=12):
        last_hours = abs(last_hours)
        timediff = datetime.now() - timedelta(hours=last_hours)
        try:
            with self.conn:
                self.cur.execute("select * from messages where time >= :timediff", {"timediff":timediff})
                this = self.cur.fetchall()
                return this
        except:
            return None

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-a", "--all", help="Fetch all values", action="store_true")
    parser.add_argument("-l", "--hours", help="Fetch only the last x hours data", type=int, default=12)
    parser.add_argument("-d", "--db", help="sqlite db infile, default=" + DEFAULT_DB_FILE, default=DEFAULT_DB_FILE)
    parser.add_argument("-o", "--outfile", help="geojson outfile, default=" + DEFAULT_OUTFILE, default=DEFAULT_OUTFILE )
    args = parser.parse_args()

    db = gen_geo(filename=args.db)

    if args.all == True:
        # return all values
        readings = db.fetch_all()
    else:
        # only return some
        args.hours = abs(args.hours)
        print(args.hours)
        readings = db.fetch_last_hours(args.hours)

    if readings != None:
        print("size of readings: " + str(len(readings)))
        #for i in readings:
        #    print (str(i['time']) + ", " + str(i['dev_id']) + ", " + str(i['rssi']))
    else:
        print("No readings found in range")

if __name__ == "__main__":
    main()
