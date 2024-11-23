#include <LedControl.h>
#include <stdio.h>
#include <stdlib.h>


#define NUM_DEVICES_PER_LC 4  // Number of devices per LedControl instance
#define NUM_LCS 3             // Number of LedControl instances
#define SCREENSPOSITIONING 1
int gameRunning = 1;


LedControl lc1 = LedControl(12, 10, 11, NUM_DEVICES_PER_LC); // (dataPin, clkPin, csPin, numDevices)
LedControl lc2 = LedControl(7, 5, 6, NUM_DEVICES_PER_LC);
LedControl lc3 = LedControl(13, 8, 9, NUM_DEVICES_PER_LC);

LedControl* ledControls[NUM_LCS] = { &lc1, &lc2, &lc3};


struct Coordinate {
    float x;
    float y;
};

typedef struct {
    struct Coordinate position;      // Overall position of the block
    struct Coordinate segments[9];   // Positions of the 9 segments relative to the block's position
    int rotation;                    // Rotation state of the block
    uint8_t currentField;
    uint8_t mapping[9];
    uint8_t visitedFields[2];
} TetrisBlock;


//uint8_t activeShifter[8] = {0};



typedef struct {
  int num_subfields;   // Number of subfields
  uint8_t **subfields; // Double pointer to uint8_t
  uint8_t *coordinateIndices;
  uint8_t activeField[9];
  

} Field;


Field* field = nullptr;

TetrisBlock* block = nullptr;


int gameRunnig = 1;

void setSubfieldToDeviceSimple(Field* field, LedControl& lc, int device, int subfieldIndex) {
    if (field == NULL || subfieldIndex < 0 || subfieldIndex >= field->num_subfields) {
        Serial.println("Invalid Field or subfield index.");
        return;
    }
    if (device < 0 || device >= NUM_DEVICES_PER_LC) {
        Serial.println("Invalid device index.");
        return;
    }

    // Update the device with the subfield pattern
    for (int col = 0; col < 8; col++) {
        lc.setColumn(device, col, field->subfields[subfieldIndex][col]);
    }
}


// Set the pattern from any subfield onto any device
void setSubfieldToDevice(Field* field, LedControl& lc, int device, int subfieldIndex) {
    if (field == NULL || subfieldIndex < 0 || subfieldIndex >= field->num_subfields) {
        Serial.println("Invalid Field or subfield index.");
        return;
    }
    if (device < 0 || device >= NUM_DEVICES_PER_LC) {
        Serial.println("Invalid device index.");
        return;
    }

    // Check if the device is in the activeField array
    bool isActive = false;
    for (int i = 0; i < 9; i++) { // Iterate through activeField (assumed size 9)
        if (field->activeField[i] == subfieldIndex) 
        {
            isActive = true;
            break;
        }
       
    }

    if (!isActive) {
        // Skip updating if the device is not active
        return;
    }

    // Update the device if it is in the activeField
    for (int col = 0; col < 8; col++) {
        lc.setColumn(device, col, field->subfields[subfieldIndex][col]);
    }
    
}

void updateDisplays(Field* field, LedControl* ledControls[], int numLCs) {
  int subField = 0;
  int device = 0;
  int bound = NUM_DEVICES_PER_LC;

  
  for (int lc = 0; lc < numLCs; lc++) {
    for (; subField < bound; subField++) {
      setSubfieldToDevice(field, *ledControls[lc], device, subField);
      device++;
    }
    device = 0;
    bound += NUM_DEVICES_PER_LC;
  }
}

// this one updates em, all the fields, the other one updates only the subfield the block is in
void updateDisplaysSimple(Field* field, LedControl* ledControls[], int numLCs) {
  int subField = 0;
  int device = 0;
  int bound = NUM_DEVICES_PER_LC;

  
  for (int lc = 0; lc < numLCs; lc++) {
    for (; subField < bound; subField++) {
      setSubfieldToDeviceSimple(field, *ledControls[lc], device, subField);
      device++;
    }
    device = 0;
    bound += NUM_DEVICES_PER_LC;
  }
}

void drawOnSubfield(Field* field, int subfieldIndex, uint8_t pattern[8]) {
  if (field == NULL || subfieldIndex < 0 || subfieldIndex >= field->num_subfields) {
    Serial.println("Invalid field or subfield index.");
    return;
  }
  for (int row = 0; row < 8; row++) {
    field->subfields[subfieldIndex][row] = pattern[row];
  }
}

void resetSingleSubfield(Field* field, int subfieldIndex) {
  if (field == NULL || subfieldIndex < 0 || subfieldIndex >= field->num_subfields) {
    Serial.println("Invalid field or subfield index.");
    return;
  }
  for (int row = 0; row < 8; row++) {
    field->subfields[subfieldIndex][row] = 0;
  }
}

void fillDevice(LedControl& lc, int device) {
    if (device < 0 || device >= NUM_DEVICES_PER_LC) {
        Serial.println("Invalid device index.");
        return;
    }

    // Fill each column of the device
    for (int col = 0; col < 8; col++) {
        lc.setColumn(device, col, 0xFF); // 0xFF turns on all LEDs in the column
    }
}

void copyFromSubfield(Field* field, int subfieldIndex, uint8_t output[8]) {
    if (field == NULL || subfieldIndex < 0 || subfieldIndex >= field->num_subfields) {
        Serial.println("Invalid field or subfield index.");
        return;
    }
    if (field->subfields[subfieldIndex] == NULL) {
        Serial.println("Subfield is NULL.");
        return;
    }
    for (int row = 0; row < 8; row++) {
        output[row] = field->subfields[subfieldIndex][row];
    }
}



void setup() {
  Serial.begin(9600);
  while (!Serial) {}
  int totalColumns = NUM_DEVICES_PER_LC * NUM_LCS;
  field = create_field(totalColumns);

  initializeDevices(ledControls, NUM_LCS, NUM_DEVICES_PER_LC);

  
  processArray(field->coordinateIndices, totalColumns);
  transformCoordinatesForMapping(field->coordinateIndices,totalColumns,NUM_DEVICES_PER_LC);
  
  
  printArray(field->coordinateIndices, 12);

  // like so far all the action is here__________________________
 
  
  struct Coordinate offsets[9] = {
      {0.0f, 0.0f},
      {1.0f, 0.0f},
      {2.0f, 0.0f},
      {0.0f, 1.0f},
      {1.0f, 1.0f},
      {2.0f, 1.0f},
      {0.0f, 2.0f},
      {1.0f, 2.0f},
      {2.0f, 2.0f}
  };
  int rotation = 0;

  
  block = setupTetrisBlock(5,16,offsets, 'l', rotation);

  print_field_binary(field);

  
 
  
}

void loop() {
  // Empty loop for this example
  
  

  unsigned long lastExecutionTime = 0; 
  const unsigned long delayTime = 250; 

  // draw the starting bottom line 
  fillRowField(field, 0, 8, NUM_LCS);
  updateDisplaysSimple(field, ledControls, NUM_LCS);
  
  while (gameRunning) 
  {
      // Game logic here
      moveBlock(block, 0, -1);
      
      mapBlockToField(field, block, 8, NUM_LCS, NUM_LCS * 8, NUM_DEVICES_PER_LC * 8);
     
      // fix of the bag with the artifact
      uint8_t deviceForSecondUpdate = block->visitedFields[1];
      // this is how to get the lc and device from the coordinate 
      uint8_t nDev = (deviceForSecondUpdate % NUM_DEVICES_PER_LC);
      uint8_t nLc = deviceForSecondUpdate / NUM_DEVICES_PER_LC; 

      // Check if the 250 ms delay has passed
      unsigned long currentTime = millis();
      if (currentTime - lastExecutionTime >= delayTime) {
          if (block->visitedFields[1] != block->visitedFields[0]) {
              setSubfieldToDeviceSimple(field, *ledControls[nLc], nDev, block->visitedFields[1]);
              
          }
          lastExecutionTime = currentTime; // Update the last execution time
      }
      //fillRowField(field, 0, 8, NUM_LCS);
      // Hardware update
      updateDisplays(field, ledControls, NUM_LCS);
      // clear 2 fields (the one the block is in and the one that is behind the block)
      
      
      unmapBlockFromField(field, block, 8, NUM_LCS, NUM_LCS * 8, NUM_DEVICES_PER_LC * 8);
      //resetSingleSubfield(field, block->visitedFields[0]);
      //resetSingleSubfield(field, block->visitedFields[1]);
      
      
      //resetSubfields(field);
  }

  free_field(field);
  free(block);
}
