CREATE TABLE IF NOT EXISTS messages(
  id INTEGER PRIMARY KEY,
  time NUMERIC NOT NULL,
  dev_id TEXT,
  dev_alt NUMERIC,
  dev_lat NUMERIC,
  dev_lon NUMERIC,
  dev_hdop NUMERIC,
  gtw_id TEXT,
  gtw_lat NUMERIC,
  gtw_lon NUMERIC,
  gtw_alt NUMERIC,
  rssi NUMERIC
);
