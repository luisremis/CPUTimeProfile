# CPUTimeProfile

Design and Implementation

We use the write function as the API for registration for application. This way, the user’s application can easily send an integer (4 bytes) with the process id. Inside the module, this integer is handled and a new element in the list is created. We use the copy_from_user method to ensure security and robustness in managing memory. 

In the timer interruption we only do three things: 

1. Check if the list is empty
2. Enqueue the work
3. Retrigger timer

This way, the routine is short and exhibits good performance. It is in the enqueue work that we traverse the list, detect anomalies (a process in the list does not exist any more), and update the cpu time. 

For setting up the timer, we used the method that converts milliseconds to jiffies. This way we can be sure that we are setting the timer with the correct interval without worrying about jiffies conversions. Since this method is implemented within the kernel, we are sure its performance is good. 

In the given function to update the cpu time, we used the cputime_to_jiffies and the jiffies_to_msec method to do the conversion and set the final cpu time in our list in milliseconds. 

We used some macros to implement a mechanism to prevent the module from triggering the timer if no process is registered. Only after a process is registered, the timer is set and in the case where the module detects that the list is empty, the timer is no longer retriggered. This makes our implementation more robust and efficient in the case of no application using it. 

Due to the simplicity of the module and its reduce functionality, we did not need to use goto statement in our code since there were no many flow irregularities to make it worth. We plan to make use of this in future MPs, were the complexity increase but the need for performance is still a priority.

We finally made a check for each allocation to have its proper deallocation at the end, in order to make sure that no memory leaks are present. 


How to run our program
To run our program, just type:
	
sh test.sh

This script file will:

1. clear the DMESG (to make it easy to read after the test)
2. clean and recompile both the module and the user app
3. load the module
4. run a small test that uses sleep and cat to get some output
5. remove the module
6. kill all the user apps (which, by design of the test, will be spinning in an infinite loop)
7. print DMESG to see the internal checkpoints of the module





Test and Output - Normal Execution

sh test.sh file content.

make
sudo dmesg -C
sudo insmod mp1.ko
cat /proc/mp1/status
echo 1 > /proc/mp1/status
cat /proc/mp1/status
./userapp &
./userapp &
./userapp &
./userapp &
./userapp &
./userapp &
./userapp &
echo "Sleeping 6 seconds"
sleep 6
echo "Reading using cat:"
cat /proc/mp1/status
echo "Sleeping 6 seconds"
sleep 6
echo "Reading using cat:"
cat /proc/mp1/status
echo "Sleeping 6 seconds"
sleep 6
echo "Reading using cat:"
cat /proc/mp1/status
sudo rmmod mp1
pkill userapp
dmesg 

screen output

Sleeping 6 seconds
New Procees PID: 12943 
New Procees PID: 12942 
New Procees PID: 12941 
New Procees PID: 12940 
New Procees PID: 12939 
New Procees PID: 12938 
New Procees PID: 12937 
Reading using cat:
PID: 12937 - CPU_time: 716 
PID: 12938 - CPU_time: 716 
PID: 12939 - CPU_time: 712 
PID: 12940 - CPU_time: 716 
PID: 12941 - CPU_time: 712 
PID: 12942 - CPU_time: 712 
PID: 12943 - CPU_time: 728 
Sleeping 6 seconds
Reading using cat:
PID: 12937 - CPU_time: 1436 
PID: 12938 - CPU_time: 1436 
PID: 12939 - CPU_time: 1432 
PID: 12940 - CPU_time: 1428 
PID: 12941 - CPU_time: 1428 
PID: 12942 - CPU_time: 1420 
PID: 12943 - CPU_time: 1440 
Sleeping 6 seconds
Reading using cat:
PID: 12937 - CPU_time: 2148 
PID: 12938 - CPU_time: 2148 
PID: 12939 - CPU_time: 2144 
PID: 12940 - CPU_time: 2152 
PID: 12941 - CPU_time: 2144 
PID: 12942 - CPU_time: 2136 
PID: 12943 - CPU_time: 2156 




DMESG OUTPUT

[155421.186693] MP1 MODULE LOADING
[155421.186728] MP1 MODULE LOADED
[155421.187646] calling status_proc_open 
[155421.188763] calling status_proc_open 
[155421.189733] calling status_proc_open 
[155421.190118] calling status_proc_open 
[155421.190471] calling status_proc_open 
[155421.190820] calling status_proc_open 
[155421.190913] receive: 12943 
[155421.192290] receive: 12942 
[155421.196246] receive: 12941 
[155421.199196] receive: 12940 
[155421.200420] receive: 12939 
[155421.204412] calling status_proc_open 
[155421.204554] receive: 12938 
[155421.208512] calling status_proc_open 
[155421.210295] receive: 12937 
[155426.204868] Running Bottom-Half work 
[155427.202447] calling status_proc_open 
[155431.217642] Running Bottom-Half work 
[155433.220166] calling status_proc_open 
[155436.230420] Running Bottom-Half work 
[155439.241838] calling status_proc_open 
[155439.302103] MP1 MODULE UNLOADING
[155439.302107] The timer is still in use...
[155439.302130] freeing item pid= 12937 
[155439.302130] freeing item pid= 12938 
[155439.302131] freeing item pid= 12939 
[155439.302132] freeing item pid= 12940 
[155439.302132] freeing item pid= 12941 
[155439.302133] freeing item pid= 12942 
[155439.302133] freeing item pid= 12943 
[155439.302138] MP1 MODULE UNLOADED




Test and Output - Removing all userapp in the middle. 

sh test.sh file content.

make
sudo dmesg -C
sudo insmod mp1.ko
cat /proc/mp1/status
echo 1 > /proc/mp1/status
cat /proc/mp1/status
./userapp &
./userapp &
./userapp &
./userapp &
./userapp &
./userapp &
./userapp &
echo "Sleeping 6 seconds"
sleep 6
echo "Reading using cat:"
cat /proc/mp1/status
echo "Sleeping 6 seconds"
sleep 6
pkill userapp
echo "Reading using cat:"
cat /proc/mp1/status
echo "Sleeping 6 seconds"
sleep 6
echo "Reading using cat:"
cat /proc/mp1/status
sudo rmmod mp1
pkill userapp
dmesg 



screen output

Sleeping 6 seconds
New Procees PID: 17286 
New Procees PID: 17282 
New Procees PID: 17284 
New Procees PID: 17283 
New Procees PID: 17285 
New Procees PID: 17281 
New Procees PID: 17280 
Reading using cat:
PID: 17280 - CPU_time: 716 
PID: 17281 - CPU_time: 712 
PID: 17282 - CPU_time: 716 
PID: 17283 - CPU_time: 716 
PID: 17285 - CPU_time: 712 
PID: 17284 - CPU_time: 716 
PID: 17286 - CPU_time: 716 
Sleeping 6 seconds
Reading using cat:
PID: 17280 - CPU_time: 1432 
PID: 17281 - CPU_time: 1428 
PID: 17282 - CPU_time: 1428 
PID: 17283 - CPU_time: 1432 
PID: 17285 - CPU_time: 1436 
PID: 17284 - CPU_time: 1428 
PID: 17286 - CPU_time: 1428 
Sleeping 6 seconds
Reading using cat:






DMESG OUTPUT

[161034.379193] MP1 MODULE LOADING
[161034.379230] MP1 MODULE LOADED
[161034.380147] calling status_proc_open 
[161034.383295] calling status_proc_open 
[161034.383299] receive: 0 
[161034.384995] calling status_proc_open 
[161034.384998] receive: 0 
[161034.388660] calling status_proc_open 
[161034.388664] receive: 0 
[161034.392078] calling status_proc_open 
[161034.392081] receive: 0 
[161034.392462] calling status_proc_open 
[161034.392465] receive: 0 
[161034.400515] calling status_proc_open 
[161034.400519] receive: 0 
[161034.404504] calling status_proc_open 
[161034.404508] receive: 0 
[161039.389150] Running Bottom-Half work 
[161040.402704] calling status_proc_open 
[161044.401921] Running Bottom-Half work 
[161046.453428] calling status_proc_open 
[161049.414748] Running Bottom-Half work 
[161049.414753] Process 17280 does not exist anymore, will be removed
[161049.414755] Process 17281 does not exist anymore, will be removed
[161049.414756] Process 17282 does not exist anymore, will be removed
[161049.414757] Process 17283 does not exist anymore, will be removed
[161049.414758] Process 17285 does not exist anymore, will be removed
[161049.414758] Process 17284 does not exist anymore, will be removed
[161049.414759] Process 17286 does not exist anymore, will be removed
[161052.461345] calling status_proc_open 
[161052.466376] MP1 MODULE UNLOADING
[161052.466379] The timer is still in use...
[161052.466401] MP1 MODULE UNLOADED




Test and Output - Removing some userapp in the middle. 

sh test.sh file content.

make
sudo dmesg -C
sudo insmod mp1.ko
cat /proc/mp1/status
echo 1 > /proc/mp1/status
cat /proc/mp1/status
./userapp &
./userapp &
echo "Sleeping 6 seconds"
sleep 6
echo "Reading using cat:"
cat /proc/mp1/status
echo "Sleeping 6 seconds"
pkill userapp
sleep 6
./userapp &
./userapp &
./userapp &
./userapp &
echo "Reading using cat:"
cat /proc/mp1/status
echo "Sleeping 6 seconds"
sleep 6
echo "Reading using cat:"
cat /proc/mp1/status
sudo rmmod mp1
pkill userapp
dmesg 




Screen output

Sleeping 6 seconds
New Procees PID: 17530 
New Procees PID: 17529 
Reading using cat:
PID: 17529 - CPU_time: 2496 
PID: 17530 - CPU_time: 2508 
Sleeping 6 seconds
Reading using cat:
Sleeping 6 seconds
New Procees PID: 17538 
New Procees PID: 17537 
New Procees PID: 17536 
New Procees PID: 17535 
Reading using cat:
PID: 17535 - CPU_time: 752 
PID: 17536 - CPU_time: 744 
PID: 17537 - CPU_time: 748 
PID: 17538 - CPU_time: 756 







DMESG OUTPUT

[161224.203514] MP1 MODULE LOADING
[161224.203545] MP1 MODULE LOADED
[161224.204446] calling status_proc_open 
[161224.205968] calling status_proc_open 
[161224.205972] receive: 0 
[161224.209658] calling status_proc_open 
[161224.209662] receive: 0 
[161229.217827] Running Bottom-Half work 
[161230.219350] calling status_proc_open 
[161234.230678] Running Bottom-Half work 
[161234.230684] Process 17529 does not exist anymore, will be removed
[161234.230686] Process 17530 does not exist anymore, will be removed
[161236.241429] calling status_proc_open 
[161236.242535] calling status_proc_open 
[161236.242539] receive: 0 
[161236.244808] calling status_proc_open 
[161236.244812] receive: 0 
[161236.249568] calling status_proc_open 
[161236.249572] receive: 0 
[161236.252993] calling status_proc_open 
[161236.252997] receive: 0 
[161239.243379] Running Bottom-Half work 
[161242.266815] calling status_proc_open 
[161242.298956] MP1 MODULE UNLOADING
[161242.298958] The timer is still in use...
[161242.298983] freeing item pid= 17535 
[161242.298985] freeing item pid= 17536 
[161242.298985] freeing item pid= 17537 
[161242.298986] freeing item pid= 17538 
[161242.298990] MP1 MODULE UNLOADED


