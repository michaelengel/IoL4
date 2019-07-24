
// Try running this script with some cli args. Example:
// ./io test/CommandLineArgs.io this is a test

"Command line arguments:\n" print
Lobby args foreach(k, v, write("'", v, "'\n"))
