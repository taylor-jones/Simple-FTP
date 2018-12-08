/*
 * Program Name: FTP Client
 * Class Name: Main
 * Author: Taylor Jones
 * Last Modified: 11/20/18
 * Description: This program acts as the client in a simple
 *  client-server FTP application.
 *
 * Sources: The following sources were used in conjunction
 *  to learn how to implement this chat server:
 *
 * - Java Multithreaded Socket Server Example
 *   https://www.youtube.com/watch?v=k5EATy-wmU4
 *
 * - "The Complete Java Master Class" on Udemy.
 *   https://www.udemy.com/java-the-complete-java-developer-course/
 *   The ideas are specifically from the "Java Networking Programming" section.
 * 
 * - There are also a few references located in the function headers of functions where they were made use of.
 *   Each of these references are located in the functions headers where they were utilized.
 */

import java.util.Scanner;


public class Main {
    private static final int MIN_VALID_PORT = 1024;
    private static final int MAX_VALID_PORT = 65535;

    private static String LIST_CMD = "-l";
    private static String LIST_ALL_CMD = "-la";
    private static String LIST_WITH_SIZE_CMD = "-ll";
    private static String LIST_RECURSIVE_CMD = "-lr";
    private static String GET_CMD = "-g";

    private static String host = "";
    private static String command = "";
    private static String filename = "";
    private static int controlPort = -1;
    private static int dataPort = -1;
    private static Scanner scanner = new Scanner(System.in);


    /**
     * Checks whether a string represents a valid port number. If so,
     * that port number is returned as an integer. Otherwise, the user is
     * continuously prompted for a valid port number until the user
     * provides a valid port number.
     *
     * @param input    A string that should represent a port #.
     * @return int     A valid port number
     */
    private static int getValidControlPort(String input) {
        int port = -1;

        do {
            try {
                port = Integer.parseInt(input.trim());
            } catch (NumberFormatException e) {
                System.out.print("Enter a valid FTP control port (" + MIN_VALID_PORT + " - " + MAX_VALID_PORT + "): ");
                input = scanner.nextLine();
            }

        } while (port < MIN_VALID_PORT || port > MAX_VALID_PORT);

        return port;
    }


    /**
     * Checks whether a string represents a valid port number, and also makes sure
     * the value isn't the same as the control port. If not valid, the user is continuously
     * prompted for a valid port until one is provided.
     *
     * @param input - A string that should represent a valid port #.
     * @return - A valid data port #.
     */
    private static int getValidDataPort(String input) {
        int port = -1;

        do {
            try {
                port = Integer.parseInt(input.trim());
            } catch (NumberFormatException e) {
                System.out.print("Enter a valid FTP data port [" + MIN_VALID_PORT + " - " + MAX_VALID_PORT + "]: ");
                input = scanner.nextLine();
            }

            if (port == controlPort) {
                System.out.println("That port is already being used as the control port.");
                port = -1;
            }

        } while (port < MIN_VALID_PORT || port > MAX_VALID_PORT);

        return port;
    }


    /**
     * Checks whether a string represents a valid FTP client command. If so,
     * the argued command is returned. Otherwise, the user is continuously
     * prompted for a valid command until one is provided.
     *
     * @param input    A string that should represent a valid command (-l or -g).
     * @return String     A valid port number
     */
    private static String getValidCommand(String input) {
        while (!input.equals(LIST_CMD) && !input.equals(LIST_ALL_CMD) && 
          !input.equals(LIST_WITH_SIZE_CMD) && !input.equals(GET_CMD) && 
          !input.equals(LIST_RECURSIVE_CMD)) {
            System.out.print("Enter a valid FTP command [" + 
            LIST_CMD + ", " + LIST_ALL_CMD + ", " + LIST_WITH_SIZE_CMD + 
            ", " + LIST_RECURSIVE_CMD + ", or " + GET_CMD + "]: ");
            input = scanner.nextLine();
        }

        return input;
    }


    /**
     * Checks whether a string contains at least one alphanumeric value.
     * If not, we'll continue to prompt the user until something
     * valid is provided.
     *
     * @param input - A string that should contain some real value.
     * @param prompt - A string to prompt the user with before reading input.
     * @return String - A string with at least one alphanumeric value.
     */
    private static String getStringWithValue(String input, String prompt) {
        while (!input.matches(".*\\w.*")) {
            System.out.print(prompt + ": ");
            input = scanner.nextLine();
        }

        return input;
    }


    /**
     * Parses the command-line arguments to make sure they're valid.
     * If not, the user will be prompted to provide valid information.
     * @param args - the command-line arguments
     */
    private static void parseArguments(String[] args) {
        int argCount = args.length;

        if (argCount == 4) {
            // the arguments should have the format:
            // <SERVER_HOST> <SERVER_PORT> <COMMAND> <DATA_PORT>
            host = getStringWithValue(args[0], "Enter a valid host name");
            controlPort = getValidControlPort(args[1]);
            command = getValidCommand(args[2]);
            dataPort = getValidDataPort(args[3]);

        } else if (argCount == 5) {
            // the arguments should have the format:
            // <SERVER_HOST> <SERVER_PORT> <COMMAND> <FILE_NAME> <DATA_PORT>
            host = getStringWithValue(args[0], "Enter a valid host name");
            controlPort = getValidControlPort(args[1]);
            command = getValidCommand(args[2]);
            filename = getStringWithValue(args[3], "Enter a valid file name");
            dataPort = getValidDataPort(args[4]);

        } else {
            System.out.println("Invalid number of arguments.");
            // just prompt for all the argument components.
            host = getStringWithValue("", "Enter a valid host name");
            controlPort = getValidControlPort("");
            command = getValidCommand("");

            if (command.equals(LIST_CMD) || command.equals(LIST_ALL_CMD) || 
            command.equals(LIST_WITH_SIZE_CMD) || command.equals(LIST_RECURSIVE_CMD)) {
                dataPort = getValidDataPort("");
            } else if (command.equals(GET_CMD)) {
                filename = getStringWithValue("", "Enter a valid file name");
                dataPort = getValidDataPort("");
            }

            System.out.print('\n'); // print a newline for readability.
        }
    }


    /**
     * Initialize the ClientRunnable object and setup signal handling.
     * Then parse the command-line arguments and run the FTP client.
     */
    public static void main(String[] args) {
        // setup signal handling
        ClientRunnable client = new ClientRunnable();
        Runtime.getRuntime().addShutdownHook(new Thread(() -> {
            client.stop(true);
        }));

        // parse the command line arguments
        parseArguments(args);

        // initialize the FTP client
        client.init(host, controlPort, command, filename, dataPort);
        client.run();
    }
}