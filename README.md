
<!-- Taylor Jones - Simple-FTP -->

This is a simple client-server FTP program that runs in the console and makes use of the sockets API.

# Project Structure

The project consists of the following:
- a *client* directory, which contains all the files used to create the FTP client.
- a *server* directory, which contains all the files used to create the FTP server.
- *ftclient*, a unix executable file used to provide an alias for starting the FTP client.
- *ftserver*, a unix executable file used to provide an alias for starting the FTP server.
- *makefile*, used to trigger compilation of project files.
- this *README.md*, used to explain the project implementation.

Also, within the "server" directory, there is an "examples" directory, which is present for the purpose of providing examples of:
- hidden files 
- directories
- nested directories
- files of different sizes 

...within the FTP server directory. These files are completely unnecesary to the base project requirements, and the program will run just fine without them, but they are provided as a convenience to allow the grader to see the effects of my extra credit implementations for the commands: -la, -ll, and -lr without the need for the grader to provide their own content to test these additional commands. However, if the grader should choose to remove or replace these files completely, the commands should work just as fine with any other content.



<br><br>

# Compiling The Program
- The FTP server is implemented in C++.
- The FTP client is implemented in Java.

There are 3 total makefiles in the project files:
- One in the root directory
- One in the "server" directory
- One in the "client" directory


<br>

## To Compile
Simply run the following command from the project's root directory:
```
make
```
This will take care of compiling both the "server" files & "client" files. Executing this command should produce the following output in the terminal:

```
Compiling FTP Server
Compiling FTP Client
All Done!
```
At that point, you're ready to run the program.


<br>

## To Clean
If you need to clean up the compiled files, you can do so by running the following command from the projects root directory:

```
make clean
``` 
This will delete the *.o files from "server" and *.class files from "client". Executing this command should produce the following output in the terminal:

```
Cleaning FTP Server
Cleaning FTP Client
All Clean!
```



<br><br>

# Running The Program
## FTP Server
Once compiled, you can run the pre-made "ftserver" file in the root directory, using the following command:
```
./ftserver <port>
```

If the script above gives you any trouble, then executing the following commands from the project's root directory will begin running the FTP Server:
```
cd server
./ftserver <port>
```

<br>

## FTP Client
Likewise, there's a pre-made "ftclient" file in the root directory, which can be run using the following command:
```
./ftclient <server-hostname> <control-port> <command> <filename> <data-port>
```

Again, if this script gives you trouble, executing the following commands from the project's root directory will begin running the FTP Client:
```
cd client
java Main <server-hostname> <control-port> <command> <filename> <data-port>
```



<br><br>

# Program Flow

1. The FTP server is started, validates the argued port, and then displays a message in the terminal stating that the server is open on the specified port. The server then waits for some FTP client to request a connection.
2. The FTP client is started on some host, and validates all client command-line arguments before requesting a connection to the FTP server.
3. Once the server acknowledges & accepts the TCP control connection, the client sends the data request.
4. The server validates the data request (which the client already validatd, but for good measure...), and, in doing so, determines if the request is valid or not. If invalid, the server responds with an error message. Otherwise, the server initiates a TCP data connection and waits for the client to accept.
5. Once the client accepts and sends a signal that it (the client) is ready to receive the response to the data request, the server either sends:
    - a directory listing (if one of the -l* commands was sent) or 
    - a multipart response about the contents of a requested file, where:
      - if the file was not found, a message about how the file wasn't found  OR
      - if the file was found, a signal that the file is ready. 
          - At this point, the client can check if the file is a duplicate or not and choose to either cancel the transmission, overwrite the existing file, or save the file received from the server as a different name.
6. Once the transmission is complete, the FTP server closes the data connection and continues to wait for any additional requests from clients (until a SIGINT is received).
7. Once the transmission is complete, the FTP client closes the TCP control connection and terminates.




<br><br>

# Additional Features

## Conditionally Colored List Items
Whenever the FTP client requests the directory listing, then FTP server uses ANSI coloring to distinguish folders from files.
- Foders are displayed in the FTP client's terminal as blue.
- Hidden files are display in the FTP client's terminal as magenta (if hidden files are requested requested).


<br>

## Retrieve files within folders
The FTP client may request files in subdirectories on the FTP server. For instance, if (on the FTP server) there exists the following files:
```
example/test.txt
test.txt
```
...then the FTP client could run:
```
./ftclient <server-hostname> <control-port> -g test.txt <data-port>
```
...to fetch the file in the FTP server's root directory or...
```
./ftclient <server-hostname> <control-port> -g example/test.txt <data-port>
```
...to fetch the file in the FTP server's "example" subdirectory.

In either case, the file is saved in the root directory of the FTP client. This is important, since the (on the FTP client) the duplicate file rules still apply as normal.


<br>

## Additional Duplicate File Options
On the FTP client, whenever a file is requested that already exists, the user is presented with a menu of 3 options:
1) Overwrite the existing file.
2) Provide a different name to use for saving the file from the FTP server.
3) Cancel fetching the file from the FTP Server.


<br>

## List Directory Options
Aside from the command "-l", there are 3 other list commands that change the output of the directory list:
- la - Additionally displays hidden files (files with a name beginning with ".").
- ll - Additionally displays the size (in bytes) of each item in the directory.
- lr - Recursively displays the items in the directory.