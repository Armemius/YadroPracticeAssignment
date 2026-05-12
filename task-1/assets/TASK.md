# Task 1: Terracraft Valrising bot algo optimization

There is a dungeon where resources are harvested, dungeon is represented as a
network of rooms indexed from `0` to `N`. Room with index `0` is the entrance.
Each room that is not the entrance has a certain amount of resources that can
be harvested. Each resource has its own value:

| Resource | Value |
| -------- | ----- |
| iron     | 7     |
| gold     | 11    |
| gems     | 23    |
| exp      | 1     |

Player has the task to gather specific resource, value of this resource in
current dungeon is doubled

On the entrance player has limited amount of food `M`. Food is consumed when
player moves from one room to another or when player harvests resource more
than once in the same room (first harvest is free). If player runs out of food
and he is not in the entrance, he dies

Dungeon map is unknown to the player, exploration rules are following:

1. When the player gets into room then it is considered visited. In visited
   rooms player can view resources and routes to adjacent rooms
2. Rooms adjacent to visited rooms are considered visible. In visible rooms
   player can see routes to adjacent rooms but cannot see resources in them
3. Only available information about rooms adjacent to the visible ones is
   their indices

## Suggested bot algo

Bot have two phases: exploration and returning

### Exploration phase

During this phase bot consumes at max `M/2` food and explores the dungeon
according to the following rules:

1. Bot goes to the adjacent room with lowest index. If all adjacent rooms are
   visited then the bot goes to the closest unvisited room with lowest index
2. In each room bot harvests resource with highest value once

### Returning phase

During this phase bot returns to the entrance according to following rules:

1. Bot chooses the shortest path to the entrance through visited rooms. If
   there are multiple paths with the same length available then bot chooses
   next room with lowest index
2. If there is more food left than needed to return to the entrance then the
   bot consumes food to harvest all resources in the current room. Starting
   from the resource with the highest value

## Task

Implement suggested bot algo to test it. Program receives dungeon map as an
input and outputs bots actions. Program should validate input data format and
if it is invalid then output the invalid input line.

All input and output parameters are separated by spaces unless stated
otherwise.

### Input data format

- First line contains integer `N`, $\text{N} \in [1, 255]$ - number of rooms in
  the dungeon without the entrance
- Following `N+1` lines contain information about rooms in the following
  format:
  1. Room index
  2. Adjacent rooms indices separated by comma
  3. Iron amount `I`, $\text{I} \in [0, 255]$
  4. Gold amount `G`, $\text{G} \in [0, 255]$
  5. Gems amount `M`, $\text{M} \in [0, 255]$
  6. Exp amount `E`, $\text{E} \in [0, 255]$
- Last line contains integer `M`, $\text{M} \in [2, 255]$ - amount of food and
  name of resource which value is doubled:
  `<amount_of_food> <iron|gold|gems|exp>`

### Output data format

- When bot moves to another room output line in format `go <room_index>`
- When bot collects resource output line in format `collect <resource_name>`
- After each action except returning to the entrance output current room state
  in format (collected resources should be represented via `_` character):
  1. `state`
  2. Current room index
  3. Iron amount
  4. Gold amount
  5. Gems amount
  6. Exp amount
- After returning to the entrance output line summary of collected resources
  in format:
  1. `result`
  2. Total iron amount
  3. Total gold amount
  4. Total gems amount
  5. Total exp amount
  6. Total value of resources gathered

### Additional task

Program should be written in C/C++. Input data is passed as a file. Output data
should be written to `result.txt`. Code should be written so that any other bot
algorithm working in the described conditions can be easily embedded and tested.

Suggest improvements to the suggested bot algo and implement them. Provide a
detailed explanation of the improvements and their impact in file
`improvements.md`

### Example

Example program execution:

```sh
task in.txt
```

Example with valid input:

`in.txt`

```txt
5
0 1,2 0 0 0 0
1 0,3 5 2 1 15
2 0,4 3 2 1 10
3 1,4 1 0 2 40
4 2,5 2 4 0 15
5 4 0 5 4 10
6 gems
```

`result.txt`

```txt
go 1
state 1 5 2 1 15
collect gems
state 1 5 2 _ 15
go 3
state 3 1 0 2 40
collect gems
state 3 1 0 _ 40
go 4
state 4 2 4 0 15
collect gold
state 4 2 _ 0 15
go 3
state 3 1 0 _ 40
go 1
state 1 5 2 _ 15
go 0
result 0 4 3 0 93
```

Example with invalid input:

`in.txt`

```txt
5
0 1,2
1 0,3 5 2 1 15
2 0,4 3 2 1 10
3 1,4 1 0 2 40
4 2|5 2 4 0 15
5 4 0 5 4 10
6 gems
```

`result.txt`

```txt
4 2|5 2 4 0 15
```
