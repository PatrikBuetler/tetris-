const uint8_t tBlock[9] = {
  0, 1, 0,
  1, 1, 1,
  0, 1, 0
};

const uint8_t iBlock[9] = {
  1, 1, 1,
  0, 0, 0,
  0, 0, 0
};

const uint8_t lBlock[9] = {
  1, 0, 0,
  1, 1, 1,
  0, 0, 0
};
const uint8_t oBlock[9] = {
  1, 1, 0,
  1, 1, 0,
  0, 0, 0
};
const uint8_t zBlock[9] = {
  1, 1, 0,
  0, 1, 1,
  0, 0, 0
};


void rotate90Clockwise(const uint8_t* src, uint8_t* dest, int N) {
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            dest[j * N + (N - 1 - i)] = src[i * N + j];
        }
    }
}

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


void setupBlockMapping(char blockType, uint8_t* mapping, int rotation) {
    int rotTimes = rotation / 90;

    switch (blockType) {
        case 't':
            memcpy(mapping, tBlock, sizeof(tBlock)); // Copy the original block
            for (int r = 0; r < rotTimes; r++) {
                uint8_t tempMapping[9];
                rotate90Clockwise(mapping, tempMapping, 3);
                memcpy(mapping, tempMapping, sizeof(tempMapping));
            }
            break;

        case 'i':
            memcpy(mapping, iBlock, sizeof(iBlock));
            for (int r = 0; r < rotTimes; r++) {
                uint8_t tempMapping[9];
                rotate90Clockwise(mapping, tempMapping, 3);
                memcpy(mapping, tempMapping, sizeof(tempMapping));
            }
            break;

        case 'o':
            // 'o' block doesn't rotate, just copy it
            memcpy(mapping, oBlock, sizeof(oBlock));
            break;

        case 'l':
            memcpy(mapping, lBlock, sizeof(lBlock));
            for (int r = 0; r < rotTimes; r++) {
                uint8_t tempMapping[9];
                rotate90Clockwise(mapping, tempMapping, 3);
                memcpy(mapping, tempMapping, sizeof(tempMapping));
            }
            break;

        case 'z':
            memcpy(mapping, zBlock, sizeof(zBlock)); // Copy the original block
            for (int r = 0; r < rotTimes; r++) {
                uint8_t tempMapping[9];
                rotate90Clockwise(mapping, tempMapping, 3);
                memcpy(mapping, tempMapping, sizeof(tempMapping));
            }
            break;

        default:
            printf("Invalid block type!\n");
            break;
    }
}



TetrisBlock* setupTetrisBlock(float posX, float posY, Coordinate offsets[], char blockType, int rotation) {
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

    // Use the separate function to handle the mapping and rotation
    setupBlockMapping(blockType, block->mapping, rotation);

    return block;
}

