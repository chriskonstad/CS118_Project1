# Building
A simple `make` should build the server

# Running
Use `./server` to run the server.  It can be run from a different directory,
but then all file paths served must be relative from the cwd from where
you ran it.

# Notes on 403s, 404s, 500s
403 errors are returned when a filetype not allowed to be served is asked for
(i.e. \*.cpp, \*.h, etc.).  Only \*.html, \*.gif, \*.jpeg, and \*.jpg are
served.

404 errors are when a filetype that is allowed is asked for but not found.

500 errors are when the server has an issue, like when it cannot find any
of the static resources (404 and 403 pages), or any other issues.
