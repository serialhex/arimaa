#include <stdlib.h>
#include <stdio.h>

#include "board.h"
#include "eval.h"

#define ELEPHANT_VALUE 20000
#define CAMEL_VALUE 5000
#define HORSE_VALUE 3000
#define DOG_VALUE 1800
#define CAT_VALUE 1500
#define RABBIT_VALUE 1000
                                                                    
int EVAL_Eval(board_t *bp, int verbose)

// Evaluation is done from gold's perspective.  At the end of the evaluation, it's adjusted to be seen from current player's perspective.

{
    // evaluation constants
    static const int piece_value[7]={0,RABBIT_VALUE,CAT_VALUE,DOG_VALUE,HORSE_VALUE,CAMEL_VALUE,ELEPHANT_VALUE};

    // variables        
    
    // utility variables
    int side_mask[OWNER_MASK];
    int row;
     
    // loop variables
    int square; 
    int side;
    int dir;
    int i;
            
    // value variables
    int value=0;
    int material_value[2]={0,0};
    int rabbit_value[2]={0,0};
     
    // how many of the pieces do the players have, and where are they?
    int elephants[2]={0,0};
    int elephant_pos[2][1];
    int camels[2]={0,0};
    int camel_pos[2][1];
    int horses[2]={0,0};
    int horse_pos[2][2];
    int dogs[2]={0,0};
    int dog_pos[2][2];
    int cats[2]={0,0};
    int cat_pos[2][2];
    int rabbits[2]={0,0};
    int rabbit_pos[2][8];
    
    // material evaluation variables
    int material[100]; // What is the piece on this square worth?

    // Initialize some evaluation stuff

    side_mask[GOLD]=0;
    side_mask[SILVER]=1;

    // Determine extra information about the board state

    for (square=11; square<=88; square++) // loop over board, initialize board state info and find where all the pieces are.
    {
        if (square%10==9) square+=2;
        switch (PIECE(square))
        {
            case ELEPHANT_PIECE :
                elephant_pos[side_mask[OWNER(square)]][elephants[side_mask[OWNER(square)]]]=square;
                elephants[side_mask[OWNER(square)]]++;
                break;
            case CAMEL_PIECE :
                camel_pos[side_mask[OWNER(square)]][camels[side_mask[OWNER(square)]]]=square;
                camels[side_mask[OWNER(square)]]++;
                break;
            case HORSE_PIECE :
                horse_pos[side_mask[OWNER(square)]][horses[side_mask[OWNER(square)]]]=square;
                horses[side_mask[OWNER(square)]]++;
                break;
            case DOG_PIECE :
                dog_pos[side_mask[OWNER(square)]][dogs[side_mask[OWNER(square)]]]=square;
                dogs[side_mask[OWNER(square)]]++;
                break;
            case CAT_PIECE :
                cat_pos[side_mask[OWNER(square)]][cats[side_mask[OWNER(square)]]]=square;
                cats[side_mask[OWNER(square)]]++;
                break;
            case RABBIT_PIECE :
                rabbit_pos[side_mask[OWNER(square)]][rabbits[side_mask[OWNER(square)]]]=square;
                rabbits[side_mask[OWNER(square)]]++;
                break;
        }
        if (OWNER(square)==GOLD || OWNER(square)==SILVER)
        {
            material[square]=piece_value[PIECE(square)];
        } else
        {
            material[square]=0;
        }
    }
    
    for (square=11; square<=88; square++)
    {    
        if (square%10==9) square+=2;
        if (OWNER(square)==GOLD || OWNER(square)==SILVER)
        {
            switch (OWNER(square))
            {
                case GOLD :
                    material_value[0]+=material[square];
                    break;
                case SILVER :
                    material_value[1]+=material[square];
                    break;
            }
        }
    }
    
    // Evaluate rabbits

    for (i=0; i<rabbits[0]; i++)
    {
        row=ROW(rabbit_pos[0][i]);
        rabbit_value[0]+=(row-1)*(row-1)*(row-1);
    }
    for (i=0; i<rabbits[1]; i++)
    {
        row=9-ROW(rabbit_pos[1][i]);
        rabbit_value[1]+=(row-1)*(row-1);
    }
    
    // Add up all the factors
           
    value+=material_value[0]-material_value[1];
    value+=rabbit_value[0]-rabbit_value[1];

    // Adjust evaluation to be from the perspective of the present player.

    if (bp->at_move==SILVER)
    {
        value=-value;
    }
    return value;
}
