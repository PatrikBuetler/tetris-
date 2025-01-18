#include <LedControl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>   // For seeding the random generator
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#define ROTATION_BUTTON_PIN 41
#define LEFT_BUTTON_PIN 42
#define RIGHT_BUTTON_PIN 43
#define START_BUTTON_PIN 44
#define DOWN_BUTTON_PIN 45
#define SPEAKER_BUTTON_PIN 7
#define SOUND_BUTTON_PIN 8

#define NUM_DEVICES_PER_LC 4  // Number of devices per LedControl instance
#define NUM_LCS 1             // Number of LedControl instances
#define SCREENSPOSITIONING 1
int gameRunning = 1;
int gameStarted = 0;
int score = 0;
Adafruit_MPU6050 mpu;
const int threshold = 1;


LedControl lc1 = LedControl(12, 10, 11, NUM_DEVICES_PER_LC); // (dataPin, clkPin, csPin, numDevices)
//LedControl lc2 = LedControl(7, 5, 6, NUM_DEVICES_PER_LC);
//LedControl lc3 = LedControl(13, 8, 9, NUM_DEVICES_PER_LC);

LedControl* ledControls[NUM_LCS] = { &lc1};//, &lc2, &lc3};


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
    char specialType[5];
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
TetrisBlock* copyBlock = nullptr;





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

void resetBlock(TetrisBlock* block, int posX, int posY)
{
  if (block == NULL) return;

  // Set the new position
  block->position.x = posX;
  block->position.y = posY;

  // Randomly select a block type
  const char blockTypes[] = {'t', 'i', 'o', 'l', 'z'};
  const int numBlockTypes = sizeof(blockTypes) / sizeof(blockTypes[0]);
  char randomBlockType = blockTypes[rand() % numBlockTypes];
  
  // Randomly select a special block type
  const int weights[] = {50, 50, 0}; // Weights for "norm", "brit", "expl" (Normal, Brittle, Explosive)
  int totalWeight = 100;
  const char specialBlockTypes[][5] = {"norm\0", "brit\0", "expl\0"}; // Normal, Brittle or Explosive
  const int numSpecialBlockTypes = sizeof(weights) / sizeof(weights[0]);
  // Generate a random number between 0 and totalWeight - 1
  int randomValue = rand() % totalWeight;
  // Determine which special block type corresponds to the random value
  int runningSum = 0;
  for (int i = 0; i < numSpecialBlockTypes; i++) {
      runningSum += weights[i];
      if (randomValue < runningSum) {
        strcpy(block->specialType, specialBlockTypes[i]); // Copy the string to block->specialType
        Serial.print("Block type generated as: ");
        Serial.println(block->specialType);
        break;
      }
  }

  // Randomly select a rotation (0, 90, 180, or 270 degrees)
  int randomRotation = (rand() % 4) * 90; // Random multiple of 90 degrees

  // Update the block's mapping and rotation using the setupBlockMapping function
  block->rotation = randomRotation;
  setupBlockMapping(randomBlockType, block->mapping, randomRotation);
}
int countBlockHeight(TetrisBlock* block) {
    int height = 0;
    for(int row=0; row<3; row++) {
      for(int col=0; col<3; col++) {
          if (block->mapping[row * 3 + col] != 0) {
            height++;
            break; // go to next row;
        }
      }
    }
    return height;
}

void explode(TetrisBlock* block, Field* field) {
  Serial.print("Explodes y=");
  Serial.println(block->position.y);
  printArray(block->mapping,9);
  int height = countBlockHeight(block);
  score += height-1; // Score is only increment once, so +1 score even though we clear more rows. 
  Serial.print("Height = ");
  Serial.println(height);
  for (int h=0; h<height; h++) {
    fillRowField(field, block->position.y+h, 8, NUM_LCS);
  }
}

void brittle(TetrisBlock* block, Field* field) {
  Serial.print("Breaking x axis between ");
  Serial.print(block->position.x);
  Serial.print(" and ");
  Serial.println(block->position.x+2);
  for (int xaxis = block->position.x; xaxis < block->position.x+3; xaxis++) {  
    int lowestRow = 0;
    int highestRow = -1;
    for (int line = 1; line < block->position.y+2; line++) {
      if (line != 0) { // We want to keep the zero row
        struct Coordinate coord;
        coord.x = xaxis;
        coord.y = line;
        if (isCoordinateSetInField(field, coord, 8, NUM_LCS,1) == 1)
        {
          if (line < block->position.y) {
            lowestRow = line;
          }
          if (line >= block->position.y) {
            highestRow = line;
          }
        }
      }
    }

    // Serial output for debugging
    Serial.print("Brittle Block! X axis = ");
    Serial.print(xaxis);
    Serial.print(" Lowest row: ");
    Serial.print(lowestRow);
    Serial.print(", Highest row: ");
    Serial.println(highestRow);
    //Serial.println(moveTopIndex);

    // move everything to the bottom
    int gapBottom = lowestRow+1;
    int gapTop = highestRow-lowestRow+1;
    for(int iters = 0; iters< (highestRow - lowestRow) + 1; iters++) {
      for (int i = lowestRow+2; i <= gapTop; i++) {
        struct Coordinate setCoord;
        setCoord.x = xaxis; 
        setCoord.y = i-1;
        struct Coordinate checkCoord;
        checkCoord.x = xaxis;
        checkCoord.y = i;
        struct Coordinate unmapCoord;
        unmapCoord.x = xaxis;
        unmapCoord.y = i;
        int toDraw = isCoordinateSetInField(field, checkCoord, 8, NUM_LCS,1);
        mapCoordinateToField(field, setCoord, 8, NUM_LCS,toDraw);
        unmapCoordinateFromField(field, unmapCoord, 8, NUM_LCS,toDraw);
      }
      gapBottom--;
      gapTop--;
      }
  }
  updateDisplaysSimple(field, ledControls, NUM_LCS);
}

void setup() {
  mpu.begin();
  pinMode(ROTATION_BUTTON_PIN, INPUT_PULLUP);
  pinMode(LEFT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
  pinMode(DOWN_BUTTON_PIN, INPUT_PULLUP);

  Serial.begin(9600);
  while (!Serial) {}
  int totalColumns = NUM_DEVICES_PER_LC * NUM_LCS; // totalColumns is Screens vertical * screens horizontal, 4 x 1 in this case
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
  int rotation = 270;

  block = setupTetrisBlock(22,24,offsets, 'l', rotation);
  copyBlock = (TetrisBlock*) malloc(sizeof(TetrisBlock));
  memcpy(copyBlock, block, sizeof(TetrisBlock));
  print_field_binary(field);
}

struct Coordinate calculateRightFallingCoord(const TetrisBlock* block, Field* field)
{
  struct Coordinate dir;
  dir.x = 0;
  dir.y = -1;

  struct Coordinate checkCoord;
  checkCoord.x = block->position.x + dir.x;
  checkCoord.y = block->position.y + dir.y;

  copyBlock->position.x = block->position.x;
  copyBlock->position.y = block->position.y;

  // make a copy of the block 

  int flag = 0;
  while(flag == 0)
  {
    if(checkCollisions(&checkCoord,field, copyBlock, 8, NUM_LCS, NUM_LCS * 8, NUM_DEVICES_PER_LC * 8)==1)
    {
      copyBlock->position.x = checkCoord.x;
      copyBlock->position.y = checkCoord.y;
      checkCoord.x = checkCoord.x + dir.x;
      checkCoord.y = checkCoord.y + dir.y;
    }
    else
    {
      flag = 1;
      checkCoord.x = copyBlock->position.x;
      checkCoord.y = copyBlock->position.y;
    }
  }

  copyBlock->position.x = block->position.x;
  copyBlock->position.y = block->position.y;
  
  return checkCoord;
}


void loop() {

  
// -----------------------------------------
  unsigned long lastExecutionTime = 0; 
  const unsigned long delayTime = 250; 

  

  int toEraseBlock = 1;
  struct Coordinate prevBlockPos;
  prevBlockPos.x = 0;
  prevBlockPos.y = 0;
  
  while (gameRunning) 
  {
    int StartEndButtonState = digitalRead(START_BUTTON_PIN);
    //Serial.println(StartEndButtonState);
    if(StartEndButtonState == HIGH && gameStarted == 1)
    {
      gameStarted = 0;
      clearAllSubfields(field);
      int posX = rand() % (NUM_LCS*NUM_DEVICES_PER_LC)+3;
      resetBlock(block, posX, 30);
      memcpy(copyBlock, block, sizeof(TetrisBlock));
      updateDisplaysSimple(field, ledControls, NUM_LCS);
    }
    else if (StartEndButtonState == HIGH && gameStarted == 0)
    {
      gameStarted = 1;
      clearAllSubfields(field);
      // draw the starting bottom line 
      fillRowField(field, 0, 8, NUM_LCS);
      
      //unFillRowField(field, 2, 8, NUM_LCS);
      updateDisplaysSimple(field, ledControls, NUM_LCS);
    }
    
    // can map the word tetris here
    if (gameStarted == 0)
    {
      struct Coordinate randCoord;
      randCoord.x = 5;
      randCoord.y = 15;

      struct Coordinate randCoord2;
      randCoord2.x = 4;
      randCoord2.y = 15;
      
      clearAllSubfields(field);
      mapCoordinateToField(field, randCoord, 8, NUM_LCS, 1);// NUM_LCS - gridwidth
      mapCoordinateToField(field, randCoord2, 8, NUM_LCS, 1);
      updateDisplaysSimple(field, ledControls, NUM_LCS);
    }

    if(gameStarted == 1)
    {
    
      // gyroscope part 
      sensors_event_t a, g, temp;
      mpu.getEvent(&a, &g, &temp);

      // basic controls
      // Read the state of each button
      int RotationButtonState = digitalRead(ROTATION_BUTTON_PIN);
      int LeftButtonState = digitalRead(LEFT_BUTTON_PIN);
      int RightButtonState = digitalRead(RIGHT_BUTTON_PIN);
      int DownButtonState = digitalRead(DOWN_BUTTON_PIN);

      // Check each button state and perform actions
      if (RotationButtonState == HIGH || abs(g.gyro.x) > threshold) { // Rotation button pressed
          uint8_t tempMapping[9]; 
          rotate90Clockwise(block->mapping, tempMapping, 3);
          memcpy(block->mapping, tempMapping, sizeof(tempMapping));
          delay(100);
          Serial.print("Rotation detected on X-axis: ");
          Serial.println(g.gyro.x);
      }

      if (LeftButtonState == HIGH || abs(g.gyro.y) > threshold) { // Left button pressed
          moveBlock(block, -1, 0);
        
      } 

      if (RightButtonState == HIGH || abs(g.gyro.z) > threshold) { // Right button pressed
          moveBlock(block, 1, 0);
      } 

      float step = 1;
      if(DownButtonState == HIGH )
      {
        step *=3;
      }

      struct Coordinate dir;
      dir.x = 0;
      dir.y = -1 * step;

      struct Coordinate checkCoord;
      checkCoord.x = block->position.x + dir.x;
      checkCoord.y = block->position.y + dir.y;

      struct Coordinate bottomCheckCoord = calculateRightFallingCoord(block, field);
      
     //Serial.print("Coordinates: (");
     //Serial.print(bottomCheckCoord.x);
     //Serial.print(", ");
     //Serial.print(bottomCheckCoord.y);
     //Serial.println(")");
      
      if(checkCollisions(&checkCoord,field, block, 8, NUM_LCS, NUM_LCS * 8, NUM_DEVICES_PER_LC * 8)==1)
      {
        toEraseBlock = 1;
        moveBlock(block, dir.x, dir.y);
      }
      else
      {
        toEraseBlock = 0;
        block->position.x = bottomCheckCoord.x;
        block->position.y = bottomCheckCoord.y;

      }
      
      mapBlockToField(field, block, 8, NUM_LCS, NUM_LCS * 8, NUM_DEVICES_PER_LC * 8);
      //mapCoordinateToField(field, bottomCheckCoord, 8, NUM_LCS, 1);
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
      
      // Hardware update
      updateDisplays(field, ledControls, NUM_LCS);
      // clearing the exact area the block was cowering
      if (toEraseBlock == 1)
      {
      unmapBlockFromField(field, block, 8, NUM_LCS, NUM_LCS * 8, NUM_DEVICES_PER_LC * 8);
      }
      // block hit bottom
      else{
        // Check and handle special block types
        if (strcmp(block->specialType, "brit") == 0) {
          Serial.print("Block stopped. Type was ");
          Serial.println(block->specialType);
          brittle(block, field);
        }
        else if (strcmp(block->specialType, "expl") == 0) {
          Serial.print("Block stopped. Type was ");
          Serial.println(block->specialType);
          // first iteration of explosion: unmap the block, it explodes only itself
          explode(block, field);
          //unmapBlockFromField(field, block, 8, NUM_LCS, NUM_LCS * 8, NUM_DEVICES_PER_LC * 8);
          
          // clear blocks in the same row, can comment out and create explosion around block too
        }

        // <-- this part happens for every block -->
        int posX = rand() % (NUM_LCS*NUM_DEVICES_PER_LC)+3;
        resetBlock(block, posX, 30);
        memcpy(copyBlock, block, sizeof(TetrisBlock));
        printBlockMapping3x3(copyBlock);
        // Full lines erasure logic
        int lowestRow = -1;  // Initialize to -1 to indicate no row has been cleared yet
        int highestRow = -1;

        for (int line = 0; line < 8 * NUM_DEVICES_PER_LC; line++) {
            if (line != 0) { // We want to keep the zero row
                if (isRowFull(field, line, 8, NUM_LCS) == 1) {
                    unFillRowField(field, line, 8, NUM_LCS);

                    // Update lowest and highest cleared rows
                    if (lowestRow == -1 || line < lowestRow) {
                        lowestRow = line; // Update lowest row
                    }
                    if (highestRow == -1 || line > highestRow) {
                        highestRow = line; // Update highest row
                    }
                }
            }
        }

        // Serial output for debugging
        //Serial.print("Lowest row: ");
        //Serial.println(lowestRow);
        //Serial.print("Highest row: ");
        //Serial.println(highestRow);

        if (highestRow!=-1)
        {
          score += 1;
          Serial.print("got one, score=");
          Serial.println(score);
          int moveTopIndex = -1;
          int thereIsOne = 0;
          for (int i = highestRow+1; i < NUM_DEVICES_PER_LC * 8; i++) 
          {
              thereIsOne = 0;
              for (int j = 0; j < NUM_LCS*8; j++)
              {
                struct Coordinate coord;
                coord.x = j;
                coord.y = i;
                if (isCoordinateSetInField(field, coord, 8, NUM_LCS,1) == 1)
                {
                  thereIsOne = 1;
                }
              }
              if(thereIsOne == 0)
              {
                moveTopIndex = i;
                break;
              }
            
          }
          //Serial.println(moveTopIndex);

          // move everything to the bottom
          
          int gapBottom = highestRow+1;
          int gapTop = moveTopIndex-1;
          for(int iters = 0; iters< (highestRow - lowestRow) + 1; iters++)
          {
            for (int i = gapBottom; i <= gapTop; i++)
            {
              for (int j = 0; j < NUM_LCS*8; j++)
              {
                struct Coordinate setCoord;
                setCoord.x = j; 
                setCoord.y = i-1;
                struct Coordinate checkCoord;
                checkCoord.x = j;
                checkCoord.y = i;
                struct Coordinate unmapCoord;
                unmapCoord.x = j;
                unmapCoord.y = i;
                int toDraw = isCoordinateSetInField(field, checkCoord, 8, NUM_LCS,1);
                mapCoordinateToField(field, setCoord, 8, NUM_LCS,toDraw);
                unmapCoordinateFromField(field, unmapCoord, 8, NUM_LCS,toDraw);
              }
            }
            gapBottom--;
            gapTop--;
          }
        }

        

        updateDisplaysSimple(field, ledControls, NUM_LCS);

      }
    }

      
  }

  free_field(field);
  free(block);
  free(copyBlock);
}
