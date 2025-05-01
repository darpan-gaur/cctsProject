# cctsProject

## Source Code
- `dag` contains the code for implementation of schediler from the paper [Efficient and Concurrent Transaction Execution
Module for Blockchains](https://people.iith.ac.in/sathya_p/index_files/PDFs/Europar2023-SawtoothSCT_Paper.pdf)
- `blockPilot` contains the code for implementation of proposers from the paper [BlockPilot: A Proposer-Validator Parallel Execution Framework
for Blockchain](https://dl.acm.org/doi/pdf/10.1145/3605573.3605621)
- `Generate Transactions` contains the code for generating transactions for experiments
- `report` contains latex files for the report

## Running the code
- To run the code in `dag` and `blockPilot`:
    - Compile: `g++ scheduler.cpp -o scheduler`
    - Execute: `./scheduler`

- Run Experiments:
    - `bash script.sh`
    - Three experiments folders are there in `Generate Transactions` : `Exp 1`, `Exp 2`, `Exp 3` each contains `script.sh`
