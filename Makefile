project-configure-fresh:
	rm -fr build
	mkdir build
	cmake -B build -S .

project-configure:
	cmake -B build -S .

project-build:
	cmake --build build

project-run-exe:
	./build/executable