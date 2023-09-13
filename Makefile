F4Client: src/F4Client.c
	gcc ./src/F4Client.c ./src/F4lib.c -o ./bin/F4Client -I./include/

F4Server: src/F4Server.c
	gcc ./src/F4Server.c ./src/F4lib.c -o ./bin/F4Server -I./include/

compile_commands.json:
	bear -- make F4Server
	bear -- make F4Client