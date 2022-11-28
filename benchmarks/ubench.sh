#!/usr/bin/bash

#--------------------------------------------------#
#
# Usage: 
# >  chmod +x ./ubench.sh
# >  ./ubench.sh 
#
#--------------------------------------------------#

# Environment 
dir_source=`git rev-parse --show-toplevel`
dir_bench="${dir_source}/benchmarks"
dir_dest="${dir_bench}/data_ubench"

# Protocols and Executables 
exec_S="${dir_source}/Shamir/benchmark"
#exec_B="${dir_source}/Blakely/benchmark"
#exec_A="${dir_source}/AsmuthBloom/benchmark"

executables=($exec_S)                         #executables=($exec_S $exec_B $exec_A)
e_size=`expr ${#executables[@]} - 1`

protocols=("Shamir")                          #protocols=("Shamir" "Blakely" "AsmuthBloom")
p_size=`expr ${#protocols[@]} - 1`

# Number of Participants
n_list=(2 4 8 16 32 64 128 256 512 999)
n_size=`expr ${#n_list[@]} - 1`

# Security Factors
l_list=(64 128 192 256 384 512)
l_size=`expr ${#l_list[@]} - 1`

####################################################
# 'MAIN'
####################################################

# Basic Environment Checks

if [ ! -d $dir_dest ]; then
   mkdir -p $dir_dest
fi

for exe in ${executables[@]};do  
   if [ ! -f $exe ]; then
      echo "ERROR: $exe does not exist"
      exit -1 
   else 
      echo "Making ${exe} executable"  
      chmod +x $exe 
   fi
done

# Run
for e_indx in $(seq 0 $e_size); do 
echo "Starting to run ${executables[$e_indx]}"
sleep 2
for n_indx in $(seq 0 $n_size); do
echo " * n=${n_list[$n_indx]}"
sleep 2 
for t_indx in $(seq $n_indx $n_size); do 
for l in ${l_list[@]}; do 

   n=${n_list[$n_indx]}
   t=${n_list[$t_indx]}
   e=${executables[$e_indx]}
   p=${protocols[$e_indx]}
   newfile="${dir_dest}/ubench-${p}-${n}_${t}_${l}"

   echo "   creating $newfile.*"
   touch ${newfile}

   # Option 1: x=`time $e $n $t $l`
   # Option 2: x=`sudo perf stat --repeat=100 $e $n $t $l`
   # Option 3: embed rstmc asm into the c code :( 

   # run each benchmark 10 times and report on the statistics
   # stdout piped to null
   # stderr carries the output of perf stat , pipe to log file for post processing 
   $e $n $t $l > ${newfile}
   echo "Run Configuration,${p},${n},${t},${l}" >> $newfile

done
done
done
   # make one file per protocol
   p="${protocols[$e_indx]}"
   log="${dir_dest}/${p}.csv"
   regex="${dir_dest}/ubench-${p}*"
   touch $log 
   echo "protocol,n,t,l,function,cycles" > $log
   cat $regex >> $log 

   # cleanup junk from it 
   sed -i '/Run Configuration/d' $log 
   sed -i '/Secret recovered/d' $log 
   sort $log -o $log 
done




