# Yadro practice assignment: Task 1

<div style="width: 100%; display: flex; justify-content: center; align-items: center;">
  <img src="./assets/demo.gif" alt="demo" height="320"/>
</div>

Description: [markdown](./assets/TASK.md) / [pdf](./assets/task.pdf)

This folder provides my solution for the first task of YADRO practice
assignment. Description of the task can be found in `TASK.md` file (english)
or `task.pdf` (russian)

## How to run

To run the solution, you need to have CMake and a C++ compiler installed on the
system. Then, you can follow these steps:

```bash
git clone https://github.com/Armemius/YadroPracticeAssignment
cd YadroPracticeAssignment/task-1
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
