/*
 * Program Name: FTP Client
 * Class Name: ClientRunnable
 * Author: Taylor Jones
 * Last Modified: 11/20/18
 * Description: The ClientRunnable class is responsible for implementing an
 *  FTP client request by initiating contact with the FTP server, passing the
 *  reqyest information, and then listening for the response to the request.
 */


import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Scanner;


public class ClientRunnable implements Runnable {
    // constants
    private String LIST_CMD = "-l";
    private String LIST_ALL_CMD = "-la";
    private String LIST_WITH_SIZE_CMD = "-ll";
    private String LIST_RECURSIVE_CMD = "-lr";
    private String GET_CMD = "-g";
    private String DONE_MSG = "\\done";
    private String GOOD_MSG = "\\good";
    private String BAD_MSG = "\\bad";
    private String READY_MSG = "\\ready\n";  // this one needs a newline, bc it's outbound.
    private String CANCEL_MSG = "\\cancel\n"; // this one needs a newline, bc it's outbound.

    // member variables
    private int controlPort;
    private int dataPort;

    private String host;
    private String command;
    private String filename;

    private Socket controlSocket = null;
    private Socket dataSocket = null;
    private ServerSocket dataReceiver = null;

    private BufferedReader reader = null;
    private PrintWriter writer = null;
    private String in;
    private Scanner scanner = new Scanner(System.in);



    /**
     * Default constructor
     */
    public ClientRunnable() {}


    /**
     * Initializes the FTP client with the parsed command-line arguments.
     * @param host - the hostname of the FTP server
     * @param controlPort - the port on which to initiate contact with the FTP server
     * @param command - the request command argument (either -l or -g)
     * @param filename - the name of the file requested (if command is -g)
     * @param dataPort - the port on which to receive the response to the data request
     */
    public void init(String host, int controlPort, String command, String filename, int dataPort) {
        this.host = host;
        this.controlPort = controlPort;
        this.command = command;
        this.filename = filename;
        this.dataPort = dataPort;
    }



    /**
     * Receives the listing of files from the server until the server sends
     * the DONE message.
     */
    private void receiveFileList() {
        System.out.println("Receiving directory structure from " + host + ":" + dataPort + '\n');

        try {
            while (!(in = reader.readLine()).equals(DONE_MSG)) {
                System.out.println(in);
            }

            System.out.print('\n');  // print an extra newline at the end for readability.
        } catch (IOException e) {
            System.out.println("Error receiving directory from FTP server: " + e.getMessage());
        }
    }



    /**
     * Checks for the existence of a duplicate file for a given filename.
     * If a duplicate is found, the user is prompted about the collision and
     * asked about either overriding the existing file or changing the name of
     * file received.
     *
     * If either:
     * - there is no duplicate file collision  OR
     * - the user chooses to override the existing file  OR
     * - the user fails to provide a valid alternative file name
     * ...then the function will return the same filename that was argued.
     *
     * If a duplicate is found and the user provides a valid alternative filename,
     *  then the alternative filename is returned.
     */
    private String getSaveName(String fName) {
        // parse the file from the path
        // https://stackoverflow.com/questions/14526260/how-do-i-get-the-file-name-from-a-string-containing-the-absolute-file-path
        File f = new File(fName);
        fName = f.getName();

        File dir = new File("./" + fName);
        if (dir.exists()) {
            // give the user the opportunity to override or change the save name.
            System.out.println("The file \"" + fName +
                "\" already exists in the directory.\n\n" +
                "What would you like to do?\n" +
                "1) Overwrite the existing file.\n" +
                "2) Change the name of the received file.\n" +
                "3) Cancel saving the received file.\n");
            System.out.print("Enter a number [1 - 3]: ");


            // force the user to make a choice
            int choice = -1;
            String input = "";

            do {
                try {
                    input = scanner.nextLine();
                    choice = Integer.parseInt(input.trim());
                } catch (NumberFormatException e) {
                    System.out.print("Please select a valid option [1 - 3]: ");
                }
            } while (choice < 1 || choice > 3);


            // update the fName based on the user's choice.
            if (choice == 3) {
                fName = CANCEL_MSG;
            } else if (choice == 2) {
                input = "";

                // get a new filename from the user.
                while (!input.matches(".*\\w.*")) {
                    System.out.print("Please enter a valid file name: ");
                    input = scanner.nextLine();
                }

                fName = input;
            }
            // Note: if choice == 1, we're just keeping the filename the same.
        }

        return fName;
    }



    /**
     * Receives the response to a file request. The response will have the following format:
     * [response type] [response content] [DONE_MSG]
     * The response type will either be: [BAD_MSG] or [GOOD_MSG].
     * - If [BAD_MSG], then there was an error retrieving the file. In this case,
     *   the [response content] will be the error message.
     * - If [GOOD_MSG], then the [response content] will be the contents of the requested file.
     * Whether successful or not, the last message received from the server will be [DONE_MSG],
     *  which is the indicator that the FTP server has completed the transmission.
     *
     * @note Resource for learning how to write to a file from the stream:
     * https://www.baeldung.com/java-write-to-file
     */
    private void receiveFileResponse() {
        FileOutputStream fileOut = null;

        try {
            // Get the first line from the FTP server, which is the response type.
            in = reader.readLine();

            // If it's an error, we'll print the response to the console.
            if (in.equals(BAD_MSG)) {
                while (!(in = reader.readLine()).equals(DONE_MSG)) {
                    System.out.println(in);
                }

            } else {
                // The server did not send an error message.
                // Determine the name of the file to write the file contents to.
                String saveToFileName = getSaveName(filename);

                if (saveToFileName.equals(CANCEL_MSG)) {
                    /* The user chose to cancel saving due to a duplicate file conflict, so send
                    the CANCEL_MSG to the server, which the server will understand as an indicator
                    to not send the file contents */
                    System.out.println("File transfer cancelled.");
                    writer.println(CANCEL_MSG);

                } else {
                    /* Setup the file output stream to using the determined save file name, and then
                      send the READY_MSG to the server to indicate that the client is ready to being
                      receiving the file contents. */
                    fileOut = new FileOutputStream(saveToFileName);

                    System.out.println("Receiving \"" + filename + "\" " +
                            (filename == saveToFileName ? "" : " as \"" + saveToFileName + "\"") +
                            " from " + host + ":" + dataPort);

                    writer.println(READY_MSG);

                    // Write each line of data received from the FTP server to the chosen file.
                    while (!(in = reader.readLine()).equals(DONE_MSG)) {
                        in += '\n';
                        byte[] b = in.getBytes();
                        fileOut.write(b);
                    }

                    System.out.println("File transfer complete.");
                }
            }

        } catch (IOException e) {
            System.out.println("Error receiving file from FTP server: " + e.getMessage());
        } finally {
            try {
                if (fileOut != null) fileOut.close();
            } catch (IOException e) {
                // do nothing
            }
        }
    }



    /**
     * Setup the data socket connection and receive the server's response
     * to the original data request.
     */
    private void receiveData() {
        try {
            dataReceiver = new ServerSocket(dataPort);
            // send a message of acknowledgement to the FTP server to indicate
            // that the client is now ready to receive the data response.
            writer.println(READY_MSG);

            // accept the connection from the FTP server & receive the response.
            dataSocket = dataReceiver.accept();
            reader = new BufferedReader(new InputStreamReader(dataSocket.getInputStream()));

            if (command.equals(LIST_CMD) || command.equals(LIST_ALL_CMD) || 
              command.equals(LIST_WITH_SIZE_CMD) || command.equals(LIST_RECURSIVE_CMD)) {
                receiveFileList();
            } else if (command.equals(GET_CMD)) {
                receiveFileResponse();
            }

        } catch (IOException e) {
            System.out.println("Error receiving data from FTP server: " + e.getMessage());
        }
    }



    /**
     * Establish initial contact with the FTP server on the control port.
     */
    private void initiateContact() {
        try {
            controlSocket = new Socket(host, controlPort);
        } catch (IOException e) {
            System.out.println("Error initiating contact with FTP server: " + e.getMessage());
            System.exit(1);
        }
    }



    /**
     * Uses the state of the member variables to build a request string which
     *  can be sent to the FTP server.
     * @return - String - the request that will be sent.
     */
    private String buildRequest() {
        String request = "";

        if (command.equals(LIST_CMD) || command.equals(LIST_ALL_CMD) || 
          command.equals(LIST_WITH_SIZE_CMD) || command.equals(LIST_RECURSIVE_CMD)) {
            request = command + " " + dataPort;
        } else if (command.equals(GET_CMD)) {
            request = command + " " + filename + " " + dataPort;
        } else {
            throw new RuntimeException("Developer error: Invalid command detected in buildRequest().");
        }

        return request;
    }



    /**
     * Sends the actual request to the FTP server, and then immediately listens for acknowledgement
     *  from the FTP server as to whether or not the request was valid. If valid, the server returns
     *  the statement "Accepted". If invalid, the server returns an error statement.
     */
    private void makeRequest() {
        try {
            // build and send the request statement
            String out = buildRequest();
            writer.println(out);

            // get acknowledgement of request valid state
            in = reader.readLine();
            if (in.equals(GOOD_MSG)) {
                // the server has validated the request format,
                // so setup the data socket and listen for the data response.
                receiveData();
            } else {
                // the server has returned an error message. print it to the console.
                System.out.println(in);
            }

        } catch (IOException e) {
            System.out.println("Error making request to FTP server: " + e.getMessage());
            System.exit(1);

        } finally {
            // close the control connection
            stop(false);
        }
    }



    /**
     * Sets up the input and output buffers and triggers contact
     * initialization with the FTP server.
     */
    public void run() {
        try {
            initiateContact();
            reader = new BufferedReader(new InputStreamReader(controlSocket.getInputStream()));
            writer = new PrintWriter(controlSocket.getOutputStream(), true);
            makeRequest();

        } catch (IOException e) {
            System.out.println("Error running FTP client: " + e.getMessage());
            System.exit(1);
        }
    }



    /**
     * Closes any open socket connections in the event of a SIGINT
     */
    public void stop(boolean isSignal) {
        try {
            /* if stop() was provoked by a signal handler,
             then try to send the CANCEL_MSG to the FTP server,
             so the server knows to close the data connection. */
            if (isSignal) {
                writer.println(CANCEL_MSG);
            }

            // Close the control socket if it's open.
            if (!controlSocket.isClosed()) {
                controlSocket.close();
                clearConsoleLine();
                System.out.println("FTP control connection with " + host + ":" + controlPort + " closed.\n");
            }

        } catch (Exception e) {
            // this means the writer wasn't setup yet.
            // no need to do anything.
        }
    }



    /**
     * Helper function to clear the current console line.
     * https://stackoverflow.com/questions/7522022/how-to-delete-stuff-printed-to-console-by-system-out-println
     */
    private void clearConsoleLine() {
        System.out.print("\033[1000D"); // Beginning of line
        System.out.print("\033[K"); // Erase line content
    }
}
