#!/usr/bin/python3

import os,sys,time
import csv
import re 

debug = True

def find_all(name, path):
    result = [] 
    for root,dirs,files in os.walk(path):
        for file in files:
            if name in file:
               result.append(os.path.join(path,file))
    return result

def parse_logs(log_list):
    
   # find (avg, dev) 
   regex = re.compile(r"(\d+.\d+)\s\+-\s(\d+.\d+)")

   results = []
   for log in log_list :
      print(f"parsing {log}")
      with open(log,'r') as f:
          x = f.readline()
          while x:
             if "Run Configuration" in x:
                 config = x.strip().split(',')
                 config = config[1:]
             if "seconds time elapsed" in x:
                 result = regex.findall(x)
             x = f.readline()
         
          data_log = []
          data_log.append(config[0])   # protocol
          data_log.append(config[1])   # n
          data_log.append(config[2])   # t
          data_log.append(config[3])   # l
          data_log.append(result[0][0])# avg
          data_log.append(result[0][1])# stddev
            
          results.append(data_log)
        
   return results 
       
#--------------------------------------------------#
#
#
#--------------------------------------------------#

# paths
path_bench= os.path.dirname(os.path.abspath(__file__))
path_data  = os.path.join(path_bench,'data')

# iterate over results
protocols = ['AsmuthBloom','Blakely','Shamir']
for protocol in protocols:

    # find data
    files = find_all(f"{protocol}-",path_data)
    data  = parse_logs(files) 

    # write data 
    fout = os.path.join(path_data,f"{protocol}_all")
    with open(fout, 'w+') as file:
        mywriter = csv.writer(file, delimiter=',')
        mywriter.writerows(data)

