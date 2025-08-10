// Initialize the UART (this is typically done via CubeMX or manually)
    // HAL_UART_Init(   huart1);  // Ensure UART is initialized for GPS

    char gpsBuffer[GPS_BUFFER_SIZE] = 0;  // Buffer for storing GPS data

    while (1) 
        GPS_Read(   huart1, gpsBuffer, GPS_BUFFER_SIZE);
        
        // If we have a completeGPGGA sentence
        if (strstr(gpsBuffer, "$GPGGA") != NULL) {
            Parse_GPGGA(gpsBuffer);  // Parse the GPGGA sentence
        }

        HAL_Delay(1000);  // Delay for a second before reading again
    }
}

// Function to read GPS data over UART
void GPS_Read(UART_HandleTypeDef *huart, char *buffer, uint16_t buffer_size) {
    uint16_t idx = 0;
    char c;

    // Read UART byte by byte
    while (1) {
        if (HAL_UART_Receive(huart, (uint8_t *)&c, 1, HAL_MAX_DELAY) == HAL_OK) {
            // Store the received character in the buffer
            buffer[idx++] = c;

            // Check for buffer overflow
            if (idx >= buffer_size) {
                idx = 0;  // Reset the index if buffer size is exceeded
            }

            // Null terminate the buffer (for safety)
            buffer[idx] = '\0';

            // If we have a complete sentence (terminated by newline)
            if (c == '\n') {
                break;
            }
        }
    }
}

// Function to parse the GPGGA sentence and extract the UTC time
void Parse_GPGGA(char *sentence) 
    char *token;
    uint8_t hour, minute, second;
    char utc_time[10];

    // Split the sentence by commas
    token = strtok(sentence, ",");  // Skip theGPGGA part

    // Skip the next fields (latitude, longitude, fix status, etc.)
    for (int i = 0; i < 1; i++) {
        token = strtok(NULL, ",");
    }

    // The UTC time is the second field after the GPGGA identifier
    token = strtok(NULL, ",");
    if (token != NULL && strlen(token) >= 6) {
        // UTC time format: hhmmss.sss
        snprintf(utc_time, sizeof(utc_time), "%s", token);  // Copy the time string

        // Extract hours, minutes, and seconds
        hour = (utc_time[0] - '0') * 10 + (utc_time[1] - '0');
        minute = (utc_time[2] - '0') * 10 + (utc_time[3] - '0');
        second = (utc_time[4] - '0') * 10 + (utc_time[5] - '0');

        // Print the UTC time in readable format
        Print_UTC_Time(hour, minute, second);
    }
}

// Function to print the extracted UTC time (hh:mm:ss format)
void Print_UTC_Time(uint8_t hour, uint8_t minute, uint8_t second) {
    char time_str[20];
    snprintf(time_str, sizeof(time_str), "UTC Time: %02d:%02d:%02d\r\n", hour, minute, second);