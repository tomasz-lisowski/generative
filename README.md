# Generative Art

# Building
When building on Windows with MinGW-w64, replace `make` in the commands below with `mingw32-make`.
1. Make sure to clone recursively so all submodules are checked out.
2. Run `make all-lib` to build all the libraries (dependencies). This step needs to be done only once.
3. Run `make` to build the common library shared by all arts.
4. Change directory to `art`.
5. Run `make` to build all arts or `make 000-test` for specific projects (where the name is just the project folder name).
