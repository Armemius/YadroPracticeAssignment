# Production facility modeling

<div style="width: 100%; display: flex; justify-content: center; align-items: center;">
  <img src="./assets/demo.gif" alt="demo" height="320"/>
</div>

Description: [pdf](./assets/task.pdf)

This folder provides my solution for the second task of YADRO practice
assignment. Description of the task can be found in `task.pdf` (russian)

## How to run

To run the solution, you need to have CMake and a C++ compiler installed on the
system. Then, you can follow these steps:

```bash
git clone https://github.com/Armemius/YadroPracticeAssignment
cd YadroPracticeAssignment/task-2
mkdir build
cd build
cmake ..
cmake --build .
```

Then you can run the program with:

```bash
./task <input_file>
```

Where `<input_file>` is the path to the input file containing the data for the
task. The program will read the input file, process the data,
and output the results to file `result.txt`
