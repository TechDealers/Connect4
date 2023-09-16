
all: clean dirs F4Client F4Server

clean:
	./utils/clean.sh

dirs:
	mkdir -p bin logs

server:
	./bin/F4Server 5 5 X O

auto:
	./bin/F4Client client '*'

F4Client: src/F4Client.c
	gcc ./src/F4Client.c ./src/F4lib.c -o ./bin/F4Client -I./include/

F4Server: src/F4Server.c
	gcc ./src/F4Server.c ./src/F4lib.c -o ./bin/F4Server -I./include/

compile_commands.json:
	bear -- make F4Server
	bear -- make F4Client

zip:
	tar -czvf assets/vr471635.cisse.VR472194.hristodor.VR497290.benbaa.tar.gz \
		--exclude '.git' \
		--exclude '.cache' \
		--exclude '.direnv' \
		--exclude '.envrc' \
		--exclude 'assets' \
		--exclude 'bin/*' \
		--exclude 'logs/*' \
		--exclude '.clang-format' \
		--exclude '.gitignore' \
		--exclude 'compile_commands.json' \
		--exclude 'flake.*' \
		./
