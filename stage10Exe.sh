#!/bin/bash 
for i in {0..1}; do 
  curl --local-port $(($i+50009)) --interface eth0 -X GET 'https://pdos.csail.mit.edu'
done

