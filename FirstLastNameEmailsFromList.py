# This script searches for emails with first.last name format in a file.
# Created with <3 by @nickvourd

import re
import argparse

def find_emails(file_path, domain):
    pattern = fr'\b[a-zA-Z]+\.[a-zA-Z]+@{domain}\b'
    with open(file_path, 'r') as file:
        text = file.read()
        emails = re.findall(pattern, text)
        return emails

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Search for emails with first.last name format in a file.')
    parser.add_argument('-f', '--file', help='Path to the text file', required=True)
    parser.add_argument('-d', '--domain', help='Domain to search for in the emails', required=True)
    parser.add_argument('-o', '--output', help='File to save the output')
    args = parser.parse_args()

    emails = find_emails(args.file, args.domain)
    if len(emails) == 0:
        print("[!] No emails found.")

    else:
        print("[+] Emails found:\n")
        for email in emails:
            print(email)

        if args.output:
            with open(args.output, 'w') as f:
                for email in emails:
                    f.write("%s\n" % email)
            print(f"[+] Emails saved to {args.output}\n")
