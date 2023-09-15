# Connect 4

Connect 4 game implementation using System V IPC in C language following **client-server** pattern.

## Bootstrap

Compile the sources by running:

```bash
make
```

then you can start the application.

The app runs in two ways, **manual** and **automatic**, manual mode requires 2 players, automatic requires only one, the other being the computer.

After startup use the _number row_ on your keyboard to place a token in the correct slot. (`from 0 to {cols} - 1`)

## Manual

Start `./bin/F4Server` with the following supported parameters:

```bash
./bin/F4Server {rows} {cols} {Player 1 Token} {Player 2 Token}
```

For example:

```bash
./bin/F4Server 5 5 X O
```

Then start two instances of `F4Client` in two different terminal windows like so:

```bash
./bin/F4Client {player name}
```

example:

```bash
./bin/F4Client my_name
```

**NOTE:** the username `computer` is system reserved.

## Automatic

Start `./bin/F4Server` with the following supported parameters:

```bash
./bin/F4Server {rows} {cols} {Player 1 Token} {Player 2 Token}
```

For example:

```bash
./bin/F4Server 5 5 X O
```

Then start one single instance of `F4Client` like so:

```bash
./bin/F4Client {player name} '*'
```

Please note the `'*'` argument.

example:

```bash
./bin/F4Client my_name '*'
```

## Other features

- **Timeout**: If a player does not make a move in `~30s` the game will go into `timeout` giving the win to the other player

- **Diagonal checks**: The game checks also diagonal cells allowing players to play as the original game

- **Before-After cleanup**: Running `make` also cleans up resources before the game starts whilst the game itself will clean the resources used on exit.

- **Debugging computer**: When running in _automatic_ mode the `STDOUT_FILENO` file descriptor of the computer process is redirected to `computer.txt`. Mostly used for debugging purposes.

- **Game endings**: Game supports: _win_, _loss_ and _tie_

## Architecture

The game consists of two main processes, the **server** and the **client**.
The server is responsible for the game logic and the client is responsible for the user interface and handles user interactions.

### Server

The server decides if each move is valid and if the game is over and it is made out of three phases:

- **Initialization**: The server creates the shared memory and the semaphores and initializes them.
- **Player connection**: The server waits for two players to connect and then starts the game.
- **Game loop**: The server waits for a player to make a move and then runs some checks to see if the move is valid and if the game is over. The server always answers to the client with a message containing the result of the move and the current state of the board. If the game is over the server sends the clients a message containing the result of the game and then runs some cleanup operations.

### Client

The client is made out of two phases:

- **Initialization**: The client connects to the server and waits for the game to start.
- **Game loop**: The client waits for the server to unlock it, asks the user for a move and then sends it to the server. It then waits for the answer of the server and processes its response. If the game is over the client prints the result of the game and then runs some cleanup operations.

## File structure

The file structure is the following:

```
├── assets
│   └── vr471635.cisse.VR472194.hristodor.VR497290.benbaa.tar.gz
├── bin
│   ├── F4Client
│   └── F4Server
├── compile_commands.json
├── flake.lock
├── flake.nix
├── include
│   ├── F4Client.h
│   ├── F4lib.h
│   └── F4Server.h
├── logs
│   └── computer.txt
├── Makefile
├── README.md
├── src
│   ├── F4Client.c
│   ├── F4lib.c
│   └── F4Server.c
└── utils
    └── clean.sh
```

Explanation:

- `include`: Folder containing the header files
- `include/F4Client.h`: Header file for the client
- `include/F4Server.h`: Header file for the server
- `include/F4lib.h`: Header file for the shared library -- contains most of the data structures used
- `src`: Folder containing the source files
- `Makefile`: Makefile used to compile the project
- `README.md`: This file
- `bin`: Folder containing the compiled binaries
- `utils/clean.sh`: Shell script used to clean up the project
- `logs/computer.txt`: File containing the output of the computer process when running in automatic mode
