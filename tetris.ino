// function to get the index of the subfield we are drawing on
uint8_t GetIndexSubField(Coordinate coord, int tileSize, int gridWidth) 
{
   
    // Calculate subfield indices
    int subfieldX = static_cast<int>(coord.x) / tileSize;
    int subfieldY = static_cast<int>(coord.y) / tileSize;

    // Determine the subfield index in the array
    int subfieldIndex = subfieldY * gridWidth + subfieldX;

    // Ensure the subfield index is within bounds
    if (subfieldIndex < 0 || subfieldIndex >= field->num_subfields) {
        // Handle error: subfield index out of bounds
        Serial.println("Error: Subfield index out of bounds.");
        return 255; // Use 255 as an invalid index indicator
    }

    // Retrieve the mapped subfield index
    uint8_t coordIndex = field->coordinateIndices[subfieldIndex];

    // Return the mapped subfield index
    return coordIndex;
    
   
}

// function for drawing on the field
void mapCoordinateToField(Field* field, Coordinate coord, int tileSize, int gridWidth, uint8_t toDraw) {
  if (toDraw)
  {
    
    uint8_t coordIndex = GetIndexSubField(coord,tileSize,gridWidth);
    
    // Calculate position within the subfield
    int posInSubfieldX = static_cast<int>(coord.x) % tileSize;
    int posInSubfieldY = static_cast<int>(coord.y) % tileSize;

    // Invert the position within the subfield
    int invertedPosX = tileSize - 1 - posInSubfieldX;
    int invertedPosY = tileSize - 1 - posInSubfieldY;

    // Set the bit at the inverted position within the subfield
    field->subfields[coordIndex][invertedPosY] |= (1 << invertedPosX);
  }
}




void mapBlockToField(Field* field, const TetrisBlock* block, int tileSize, int gridWidth, int fieldWidth, int fieldHeight) 
{
    // Map the center position of the block
    //mapCoordinateToField(field, block->position, tileSize, gridWidth);
    wrapCoord(&block->position, fieldWidth, fieldHeight);
    // Map each segment relative to the block's center position
    for (int i = 0; i < 9; i++) {
        struct Coordinate segmentCoord;
        segmentCoord.x = block->position.x + block->segments[i].x;
        segmentCoord.y = block->position.y + block->segments[i].y;
        wrapCoord(&segmentCoord, fieldWidth, fieldHeight);
        field->activeField[i] = GetIndexSubField(segmentCoord, tileSize,gridWidth);
        mapCoordinateToField(field, segmentCoord, tileSize, gridWidth, block->mapping[i]);
    }
    
}

void unmapCoordinateFromField(Field* field, Coordinate coord, int tileSize, int gridWidth, uint8_t toClear) {
  if (toClear) {
    uint8_t coordIndex = GetIndexSubField(coord, tileSize, gridWidth);

    // Calculate position within the subfield
    int posInSubfieldX = static_cast<int>(coord.x) % tileSize;
    int posInSubfieldY = static_cast<int>(coord.y) % tileSize;

    // Invert the position within the subfield
    int invertedPosX = tileSize - 1 - posInSubfieldX;
    int invertedPosY = tileSize - 1 - posInSubfieldY;

    // Clear the bit at the inverted position within the subfield
    field->subfields[coordIndex][invertedPosY] &= ~(1 << invertedPosX); // Clear the bit
  }
}

void unmapBlockFromField(Field* field, const TetrisBlock* block, int tileSize, int gridWidth, int fieldWidth, int fieldHeight) {
    // Wrap the block's position within the field dimensions
    wrapCoord(&block->position, fieldWidth, fieldHeight);

    // Unmap each segment relative to the block's center position
    for (int i = 0; i < 9; i++) {
        struct Coordinate segmentCoord;
        segmentCoord.x = block->position.x + block->segments[i].x;
        segmentCoord.y = block->position.y + block->segments[i].y;
        wrapCoord(&segmentCoord, fieldWidth, fieldHeight);

        // Clear the bit corresponding to this segment
        unmapCoordinateFromField(field, segmentCoord, tileSize, gridWidth, block->mapping[i]);
    }
}




void fillRowField(Field* field, float y, int tileSize, int gridWidth) {
  
    for(int i = 0; i<(tileSize*gridWidth); i++)
    {
      struct Coordinate coord;
      coord.x = i;
      coord.y = y;
      mapCoordinateToField(field, coord, tileSize, gridWidth, 1);
    }
}

bool isCoordinateSetInField(Field* field, Coordinate coord, int tileSize, int gridWidth, uint8_t toCheck) {
    if (toCheck) {
        uint8_t coordIndex = GetIndexSubField(coord, tileSize, gridWidth);

        // Calculate position within the subfield
        int posInSubfieldX = static_cast<int>(coord.x) % tileSize;
        int posInSubfieldY = static_cast<int>(coord.y) % tileSize;

        // Invert the position within the subfield
        int invertedPosX = tileSize - 1 - posInSubfieldX;
        int invertedPosY = tileSize - 1 - posInSubfieldY;

        // Check if the bit is set at the inverted position
        return (field->subfields[coordIndex][invertedPosY] & (1 << invertedPosX)) != 0;
    }
    return false;
}


int checkCollisions(TetrisBlock* block, Field* field, Coordinate* coord, int fieldWidth, int fieldHeight, int tileSize, int gridWidth) {
    // Wrap the block's position within the field dimensions
    wrapCoord(&block->position, fieldWidth, fieldHeight);

    // Check each segment relative to the block's center position
    for (int i = 0; i < 9; i++) {
        if (block->mapping[i]) { // Check only active pixels
            struct Coordinate segmentCoord;
            segmentCoord.x = block->position.x + block->segments[i].x;
            segmentCoord.y = block->position.y + block->segments[i].y;
            wrapCoord(&segmentCoord, fieldWidth, fieldHeight);

            // Use the isCoordinateSetInField function to check for collision
            if (isCoordinateSetInField(field, segmentCoord, tileSize, gridWidth, 1)) {
                return 0; // Collision detected
            }
        }
    }

    return 1; // No collisions
}



