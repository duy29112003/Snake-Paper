compile:
	g++ -Iinclude -Iinclude/sdl -Iinclude/headers -Llib -o Main src/*.cpp -lmingw32 -lSDL2main -lSDL2 -lSDL2_image

run:
	g++ -Iinclude -Iinclude/sdl -Iinclude/headers -Llib -o Main src/*.cpp -lmingw32 -lSDL2main -lSDL2 -lSDL2_image
	./Main.exe

run-only:
	./Main.exe