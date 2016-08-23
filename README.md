# jansson-testing
Test of the Jansson JSON library for C.
This is a command line utility that will print a list of commits
to a given GitHub repository owned by a given user.

### Dependencies
The jansson library is required (obviously), but also cURL for making API requests.

`sudo apt-get install libjansson-dev libcurl4-openssl-dev`

### Build It
A Makefile is included, so just run `make main`

### Run it
Usage: `main <user> <repo>`
