# cdimport
A quick application that I have been working on to add my music CD collection to a database.

# Building
To build the project, follow these steps
```bash
mkdir build
cd build
cmake ..
make -j
```

To make the documentation, execute this command in the `build` folder that you created above

```bash
make docs
```

# Dependencies
This project makes use a command line tools that are executed via the `system()` function.
It is also dependend upon a variety of development libraries. Below is list of Ubuntu
dependencies. *Warning*, it may not be complete. Sorry.

* qtbase5-dev-tools
* qt5-default
* qtbase5-dev
* libpq-dev
* cd-discid
* abcde
