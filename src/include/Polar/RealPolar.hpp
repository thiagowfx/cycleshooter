#ifndef _REALPOLAR_HPP_
#define _REALPOLAR_HPP_

#include <cerrno>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <stdexcept>

#include "AbstractPolar.hpp"
#include "ConfigManager.hpp"
#include "Logging.hpp"

namespace Cycleshooter {
/**
 * @brief Reads a heart rate value from a serial device connected to the specified serial port.
 */
class RealPolar : public AbstractPolar {
    /**
     * Serial port file descriptor.
     */
    int serialDescriptor = -1;

    /**
     * Original attributes of the serial port.
     * Saved temporarily to be restored in the end.
     */
    struct termios originalTTYAttributes;

    /**
     * Open the serial port in the specified path.
     */
    void openSerialPort(const char* deviceFilePath) {
        LOG("Polar: opening serial port");

        // open the serial port
        if ((serialDescriptor = open(deviceFilePath, O_RDWR | O_NOCTTY )) == -1) {
            LOG_FATAL("Polar: Error opening serial port %s -- %s (%d)", deviceFilePath, strerror(errno), errno);
        }

        // prevent other processes from opening the serial port
        if (ioctl(serialDescriptor, TIOCEXCL) == -1) {
            LOG_FATAL("Polar: Error setting TIOCEXCL on port %s -- %s (%d)", deviceFilePath, strerror(errno), errno);
        }

        // get the current serial port options and save them to restore on exit
        if (tcgetattr(serialDescriptor, &originalTTYAttributes) == -1) {
            LOG_FATAL("Polar: Error getting tty attributes on port %s -- %s (%d)", deviceFilePath, strerror(errno), errno);
        }

        // serial port configuration options
        struct termios options = originalTTYAttributes;

        // set raw input (non-canonical) mode, with reads blocking until either
        // a single character has been received, or a one-second timeout expires
        cfmakeraw(&options);
        options.c_cc[VMIN] = 1;
        options.c_cc[VTIME] = 10;

        // set the baud rate and word length
        cfsetspeed(&options, B9600);
        options.c_cflag |= (CS8);

        // cause new options to take effect immediately
        if (tcsetattr(serialDescriptor, TCSANOW, &options) == -1) {
            LOG_FATAL("Polar: Error setting tty attributes on %s -- %s (%d)", deviceFilePath, strerror(errno), errno);
        }
    }

    /**
     * Close the serial port.
     */
    void closeSerialPort() {
        LOG("Polar: closing serial port");

        // block until all written output has been sent from the device
        if (tcdrain(serialDescriptor) == -1) {
            LOG_FATAL("Polar: Error waiting for drain - %s (%d)", strerror(errno), errno);
        }

        // reset the serial port back to the state in which we found it
        if (tcsetattr(serialDescriptor, TCSANOW, &originalTTYAttributes) == -1) {
            LOG_FATAL("Polar: Error restoring tty attributes - %s (%d)", strerror(errno), errno);
        }

        // close the port
        close(serialDescriptor);
    }

    /**
     * Send a command to get the specified number of heart rate values.
     */
    void sendGetHeartRate(int numEntries = 1) {
        // clamping
        numEntries = std::max(0, numEntries);
        numEntries = std::min(32, numEntries);

        // array sized to hold the largest command string
        char sendCommand[8];

        // build the command string
        // note: "\015" is the carriage return character ('\r')
        int cmdLength = sprintf(sendCommand, "G%0d\015", numEntries);

        // send the command string
        if (write(serialDescriptor, sendCommand, cmdLength) != cmdLength) {
            LOG_FATAL("Polar: Error sending the command string %s -- %s (%d)", sendCommand, strerror(errno), errno);
        }
    }

    /**
     * Read a response string back from the HRMI.
     */
    void getResponseString(char* responseString) {
        char b[2];
        int i = 0;

        do {
            // read one char at a time
            int n = read(serialDescriptor, b, 1);

            if (n == -1) {
                LOG_FATAL("Polar: Error getting the response string %s -- %s (%d)", responseString, strerror(errno), errno);
            }

            // no chars available for reading right now
            if (n == 0) {
                // wait a little bit before trying again
                usleep(READING_RETRY_TIME_MS * 1e3);
                continue;
            }

            // store the character
            responseString[i++] = b[0];

            // repeat until we see the <CR> ('\r') character or exceed the buffer
        } while ((b[0] != 0x0D) && (i < MAX_STRING_RESPONSE));

        // null terminate the string (replace the <CR>)
        responseString[i - 1] = 0;
    }

    /**
     * Maximum number of characters that will be read until a '\r' is found.
     */
    const int MAX_STRING_RESPONSE = ConfigManager::instance().getInt("RealPolar.max_string_response");

    /**
     * Time to wait before trying to read from the serial port again if no chars available.
     */
    const int READING_RETRY_TIME_MS = ConfigManager::instance().getInt("RealPolar.reading_retry_time_ms");

public:
    /**
     * @brief RealPolar Construct a new RealPolar object.
     * @param deviceFilePath Path to the serial port (e.g. "/dev/ttyUSB0")
     */
    RealPolar(const char* deviceFilePath) :
        AbstractPolar()
    {
        openSerialPort(deviceFilePath);
    }

    /**
     * Cleanly close the serial port.
     */
    virtual ~RealPolar() {
        closeSerialPort();
    }

    /**
     * Get the instantaneous heart rate value from the HRMI.
     */
    virtual void updateHeartRate() {
        // send a get heart Rate command requesting history buffer entries
        sendGetHeartRate();

        // response string
        std::vector<char> rspBytes(MAX_STRING_RESPONSE);
        getResponseString(&rspBytes[0]);

        // discard the two first values (the heart rate is the third one)
        int heartRate;
        sscanf(&rspBytes[0], "%*u %*u %u", &heartRate);

        updateStatistics(heartRate);
    }
};

}

#endif
