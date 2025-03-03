#!/usr/bin/python3
import datetime

# The given filetime value
filetime_value = 133850376042623425 # change this

# Convert to datetime
datetime_value = datetime.datetime.fromtimestamp((filetime_value - 116444736000000000) / 10000000)

# Print the result in a readable format
print(datetime_value.strftime('%Y-%m-%d %H:%M:%S'))
