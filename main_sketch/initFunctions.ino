// Function to create the Field structure dynamically
Field* create_field(int num_subfields) {

  Field *field = (Field *)malloc(sizeof(Field));
  if (field == NULL) {
    Serial.println("Failed to allocate memory for Field");
    while (1);
  }
  field->num_subfields = num_subfields;
  // initialize coordinates for mapping
  field->coordinateIndices = (uint8_t *)malloc(num_subfields * sizeof(uint8_t));
  for (int i = 0; i < field->num_subfields; i++)
  {
    field->coordinateIndices[i] = (uint8_t)i;
  }

  field->subfields = (uint8_t **)malloc(num_subfields * sizeof(uint8_t *));
  if (field->subfields == NULL) {
    Serial.println("Failed to allocate memory for subfields array");
    free(field);
    while (1);
  }

  for (int i = 0; i < num_subfields; i++) {
    field->subfields[i] = (uint8_t *)malloc(8 * sizeof(uint8_t));
    if (field->subfields[i] == NULL) {
      Serial.print("Failed to allocate memory for subfield ");
      Serial.println(i);
      for (int j = 0; j < i; j++) {
        free(field->subfields[j]);
      }
      free(field->subfields);
      free(field);
      while (1);
    }
    memset(field->subfields[i], 0, 8 * sizeof(uint8_t));
  }

  for (int i = 0; i < 9; i++) {
        field->activeField[i] = -1;
    }
  return field;
}

void free_field(Field *field) {
    if (field == NULL) {
        return; // Nothing to free
    }

    // Free the memory allocated for subfields
    if (field->subfields != NULL) {
        for (int i = 0; i < field->num_subfields; i++) {
            if (field->subfields[i] != NULL) {
                free(field->subfields[i]);
            }
        }
        free(field->subfields);
    }

    // Free the memory allocated for coordinate indices
    if (field->coordinateIndices != NULL) {
        free(field->coordinateIndices);
    }

    // Free the Field structure itself
    free(field);
}

// Function to reverse a subarray from index 'start' to 'end'
void reverseSubarray(uint8_t* array, int start, int end) {
    while (start < end) {
        int temp = array[start];
        array[start] = array[end];
        array[end] = temp;
        start++;
        end--;
    }
}

// Function to process the array as described
void processArray(uint8_t* array, int length) {
    int groupSize = 4;

    // Iterate over the array in steps of 'groupSize'
    for (int i = 0; i < length; i += groupSize) {
        int end = i + groupSize - 1;

        // Ensure 'end' does not exceed array bounds
        if (end >= length) {
            end = length - 1;
        }

        // Reverse the current subarray
        reverseSubarray(array, i, end);
    }
}

// Function to rearrange elements into columns
void transformCoordinatesForMapping(uint8_t* array, int length, int groupSize) {
    int columns = (length + groupSize - 1) / groupSize; // Number of columns
    int rearranged[length];
    int index = 0;

    // Fill the rearranged array column by column
    for (int col = 0; col < groupSize; col++) {
        for (int row = 0; row < columns; row++) {
            int idx = row * groupSize + col;
            if (idx < length) {
                rearranged[index++] = array[idx];
            }
        }
    }

    // Copy the rearranged array back to the original
    for (int i = 0; i < length; i++) {
        array[i] = rearranged[i];
    }
}

void printArray(uint8_t* array, int length) {
    Serial.print("[");
    for (int i = 0; i < length; i++) {
        Serial.print(array[i]);
        if (i < length - 1) {
            Serial.print(", ");
        }
    }
    Serial.println("]");
}

// Function to print the Field with binary representations
void print_field_binary(Field *field) {
  if (field == NULL) {
    Serial.println("Field is NULL.");
    return;
  }

  int subfields_per_column = NUM_DEVICES_PER_LC;
  int total_subfields = field->num_subfields;
  int num_columns = (total_subfields + subfields_per_column - 1) / subfields_per_column;

  for (int col = 0; col < num_columns; col++) {
    int start_subfield = col * subfields_per_column;
    int end_subfield = start_subfield + subfields_per_column;
    if (end_subfield > total_subfields) {
      end_subfield = total_subfields;
    }

    Serial.print("Column Group ");
    Serial.println(col);

    for (int row = 0; row < 8; row++) {
      for (int subfield = start_subfield; subfield < end_subfield; subfield++) {
        uint8_t value = field->subfields[subfield][row];
        for (int bit = 7; bit >= 0; bit--) {
          Serial.print((value >> bit) & 1);
        }
        Serial.print(" ");
      }
      Serial.println();
    }
    Serial.println();
  }
}



void initializeDevices(LedControl* ledControls[], int numLcs, int numDevicesPerLc) {
    for (int device = 0; device < numDevicesPerLc; device++) {
        for (int i = 0; i < numLcs; i++) {
            ledControls[i]->shutdown(device, false); // Enable the device
            ledControls[i]->setIntensity(device, 8); // Set intensity
            ledControls[i]->clearDisplay(device);    // Clear the display
        }
    }
}

void clearDevices(LedControl* ledControls[], int numLcs, int numDevicesPerLc) {
    for (int device = 0; device < numDevicesPerLc; device++) {
        for (int i = 0; i < numLcs; i++) {
            ledControls[i]->clearDisplay(device); // Clear the display for each device
        }
    }
}


void resetSubfields(Field *field) {
    if (field == NULL || field->subfields == NULL) {
        return; // Nothing to reset if the Field or subfields are NULL
    }

    // Iterate over each subfield and set all elements to zero
    for (int i = 0; i < field->num_subfields; i++) {
        if (field->subfields[i] != NULL) {
            memset(field->subfields[i], 0, sizeof(uint8_t) * field->num_subfields); // Assuming each subfield has size 10
        }
    }
}

void clearAllSubfields(Field* field) {
    if (!field || !field->subfields) {
        return; // Nothing to clear if the Field or subfields pointer is NULL
    }

    // Iterate over each subfield
    for (int i = 0; i < field->num_subfields; i++) {
        if (field->subfields[i] != NULL) {
            // If each subfield is 8 bytes, set all 8 to zero
            memset(field->subfields[i], 0, 8 * sizeof(uint8_t));
        }
    }
}

uint8_t findLeastPrevailing(uint8_t arr[], int size) {
    int count[256] = {0}; // Array to store counts for each possible uint8_t value (0-255)

    // Count occurrences of each number
    for (int i = 0; i < size; i++) {
        count[arr[i]]++;
    }

    // Find the minimum non-zero count and corresponding number
    int minCount = size;
    uint8_t leastPrevailing = 255; // Default to an invalid value

    for (int i = 0; i < 256; i++) {
        if (count[i] > 0 && count[i] < minCount) {
            minCount = count[i];
            leastPrevailing = (uint8_t)i;
        }
    }

    return leastPrevailing;
}

uint8_t findMostPrevailing(uint8_t arr[], int size) {
    int count[256] = {0}; // Array to store counts for each possible uint8_t value (0-255)

    // Count occurrences of each number
    for (int i = 0; i < size; i++) {
        count[arr[i]]++;
    }

    // Find the maximum count for values not equal to 255
    int maxCount = 0;
    uint8_t mostPrevailing = 0; // Default to an arbitrary valid value

    for (int i = 0; i < 256; i++) { // Consider all values
        if (i != 255 && count[i] > maxCount) { // Exclude 255
            maxCount = count[i];
            mostPrevailing = (uint8_t)i;
        }
    }

    return mostPrevailing;
}

void printBlockMapping3x3(const TetrisBlock* block) {
    if (block == NULL) {
        Serial.println("Block pointer is NULL.");
        return;
    }

    Serial.println("Block mapping (3x3):");
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            int index = row * 3 + col;
            // If mapping[index] is non-zero, print 'X'; otherwise '.'
            if (block->mapping[index] != 0) {
                Serial.print("X ");
            } else {
                Serial.print(". ");
            }
        }
        Serial.println(); // new line after each row
    }
    Serial.println(); // extra blank line at the end
}
