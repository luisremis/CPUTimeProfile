make
sudo dmesg -C
sudo insmod mp1.ko
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
pkill userapp
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
