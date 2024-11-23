void wrapCoord(Coordinate *coord, int fieldWidth, int fieldHeight) {
    if (block == NULL) return; // Ensure the block pointer is not NULL

    // Wrap the block's position using modulo
    coord->x = ((int)coord->x % fieldWidth + fieldWidth) % fieldWidth;
    coord->y = ((int)coord->y % fieldHeight + fieldHeight) % fieldHeight;
}


void moveBlock(TetrisBlock *block, int dx, int dy) 
{
    if (block == NULL) return; // Ensure the block pointer is not NULL

    // Update the position of the block
    block->position.x += dx;
    block->position.y += dy;

    uint8_t subFIndex = GetIndexSubField(block->position, 8, NUM_LCS);
    block->visitedFields[0] = block->currentField;
    if(block->currentField != subFIndex)
    {
      block->currentField = subFIndex;
      block->visitedFields[1] = block->visitedFields[0];
    }
    
}



// Function to dynamically allocate and set up a TetrisBlock
TetrisBlock* setupTetrisBlock(float posX, float posY, Coordinate offsets[], int rotation) {
    if (offsets == NULL) return NULL; // Ensure offsets are valid

    // Allocate memory for the TetrisBlock
    TetrisBlock* block = (TetrisBlock*)malloc(sizeof(TetrisBlock));
    if (block == NULL) {
        printf("Failed to allocate memory for TetrisBlock\n");
        return NULL;
    }

    // Set the block's overall position
    block->position.x = posX;
    block->position.y = posY;

    // Copy the offsets into the block's segments
    for (int i = 0; i < 9; i++) {
        block->segments[i] = offsets[i];
    }

    // Set the rotation state
    block->rotation = rotation;

    return block;
}

