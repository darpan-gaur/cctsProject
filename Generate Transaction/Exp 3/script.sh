g++ /home/darpan/Desktop/Desktop/8thSem/CS5280/cctsProject/dag/scheduler.cpp -o dag
g++ /home/darpan/Desktop/Desktop/8thSem/CS5280/cctsProject/blockPilot/scheduler.cpp -o blockpilot

# make logFile.txt
if [ -f logFile.txt ]; then
    rm logFile.txt
fi
echo "threads time abort/time" > logFile.txt

# print names of all files in current directory having 'transaction'
for t in {1..5}; do
    threads=$((2**t))
    echo "${threads} 3000 transactions_3000_40.txt" > input.txt
    echo "${threads} dag" >> logFile.txt
    cat input.txt
    echo "running dag"
    for i in {1..5}; do
        ./dag >> logFile.txt
    done
    echo "running blockpilot"
    echo "${threads} blockpilot" >> logFile.txt
    for i in {1..5}; do
        ./blockpilot >> logFile.txt
    done
    
done

rm input.txt
rm dag
rm blockpilot
rm output_DAG.txt
rm output_OCC.txt