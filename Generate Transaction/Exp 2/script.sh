g++ /home/darpan/Desktop/Desktop/8thSem/CS5280/cctsProject/dag/scheduler.cpp -o dag
g++ /home/darpan/Desktop/Desktop/8thSem/CS5280/cctsProject/blockPilot/scheduler.cpp -o blockpilot

# make logFile.txt
if [ -f logFile.txt ]; then
    rm logFile.txt
fi
echo "dp time abort/time" > logFile.txt

# print names of all files in current directory having 'transaction'
for file in *; do
    if [[ "$file" == *transaction* ]]; then
        # echo "$file"
        # $file is of type transactions_1000_40.txt, print middle number
        dp=${file:18:2}
        echo "16 3000 ${file}" > input.txt
        echo "${dp} dag" >> logFile.txt
        cat input.txt
        echo "running dag"
        for i in {1..5}; do
            ./dag >> logFile.txt
        done
        echo "running blockpilot"
        echo "${dp} blockpilot" >> logFile.txt
        for i in {1..5}; do
            ./blockpilot >> logFile.txt
        done
    fi
done

rm input.txt
rm dag
rm blockpilot
rm output_DAG.txt
rm output_OCC.txt