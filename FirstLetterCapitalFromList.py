# This script capitalizes the first letter of all words in a list.
# Created with <3 by @nickvourd

import argparse

def capitalize_words(input_file, output_file=None):
    with open(input_file, 'r') as file:
        lines = file.readlines()
        lines = [line.capitalize() for line in lines]

    if output_file:
        with open(output_file, 'w') as file:
            file.writelines(lines)
    else:
        with open(input_file, 'w') as file:
            file.writelines(lines)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Capitalize the first letter of all words in a list.')
    parser.add_argument('-f', '--file', type=str, help='The list file to be capitalized', required=True)
    parser.add_argument('-o', '--output', type=str, help='The file path to save the capitalized content', required=False)
    args = parser.parse_args()
    
    file_path = args.file
    output_path = args.output
    capitalize_words(file_path, output_path)
    if output_path:
        print(f"[+] You can find the capitalized content in {output_path}.")
    else:
        print("[+] File successfully updated with capitalized words.")
