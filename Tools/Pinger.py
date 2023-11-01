# This script is a custom ping tool to keep alive the connection
# Created with <3 by @nickvourd
# Dependencies: pip3 install ping3

import time
import argparse
from ping3 import ping

DEFAULT_PING_INTERVAL = 599

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Ping a computer.")
    parser.add_argument("-c", "--computer", help="Set the IP address / Computer name to ping", required=True)
    parser.add_argument("-i", "--interval", type=int, help="Set the ping interval in seconds", default=DEFAULT_PING_INTERVAL)
    args = parser.parse_args()

    ip_address = args.computer
    ping_interval = args.interval

    try:
        while True:
            # Perform a ping
            response_time = ping(ip_address)

            if response_time is not None:
                print(f"Ping response from {ip_address}: {response_time} ms")
            else:
                print(f"No response from {ip_address}")

            # Wait for the specified interval
            time.sleep(ping_interval)

    except KeyboardInterrupt:
        print("Ping script terminated.")
