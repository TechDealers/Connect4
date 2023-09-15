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
