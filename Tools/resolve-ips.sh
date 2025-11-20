#!/usr/env bash

# Change with your list

while read -r domain; do
    ip=$(dig +short "$domain" | head -n 1)
    echo "$domain -> $ip"
done < list.txt
