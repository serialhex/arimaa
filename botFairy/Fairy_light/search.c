#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <time.h>

#include "search.h"
#include "board.h"
#include "eval.h"
#include "hash.h"

#define MAX_PV_LENGTH 100

static unsigned long long int nodes, total_nodes;
static unsigned long long int evals, total_evals;
static unsigned long long int sum_eval_depths;
static int depth, max_depth;
static move_t pv[MAX_PV_LENGTH][MAX_PV_LENGTH];
static int pv_length[MAX_PV_LENGTH];
static int hash_hit[MAX_PV_LENGTH];
static long long int start_time;
static long long int allocated_time;
static unsigned long long int start_hashkey;

void Generate_Hash_Move(hash_entry_t *hp, move_t *mp);

void Show_Search_Info(board_t *bp, int ply_finished, int depth, int value)
{
    long long int elapsed_time;
    int pv_steps[MAX_PV_LENGTH];
    int i;
    
    elapsed_time=clock()-start_time;
    if (ply_finished)
    {
        if (evals==0)
        {
            sprintf(message,"===%3d/    /  ",depth,(double)sum_eval_depths/evals,max_depth);
            BOARD_Message();
        } else
        {
            sprintf(message,"===%3d/%4.1f/%2d",depth,(double)sum_eval_depths/evals,max_depth);
            BOARD_Message();
        }
    } else
    {
        sprintf(message,"   %3d........",depth);
        BOARD_Message();
    }
    sprintf(message,"%7d %9d %9d",value,(long int)(total_nodes+nodes),(long int)(total_evals+evals));
    BOARD_Message();
    sprintf(message," %6.2f ",(double)(elapsed_time)/CLOCKS_PER_SEC);
    BOARD_Message();
    pv_steps[0]=1;
    for (i=0; i<pv_length[0]; i+=pv[0][i].steps)
    {
        if (hash_hit[0]==i)
        {
            sprintf(message,"<HT>");
            BOARD_Message();
        } else
        {
            sprintf(message," ");
            BOARD_Message();
        }
        BOARD_Print_Move(&pv[0][i]);
        pv_steps[i+pv[0][i].steps]=pv[0][i].steps;
    }
    if (hash_hit[0]==i)
    {
        sprintf(message," <HT>");
        BOARD_Message();
    }
    sprintf(message,"\n");
    BOARD_Message();
}

int SEARCH_Start_Search(board_t *bp, move_t ml[4])
{
    int i, j, k;
    int steps=0;
    int value, old_value;
    int pv_steps[MAX_PV_LENGTH];
    long long int elapsed_time;
    long long int remaining_time;
    int number_of_steps;
    int depth_limit;
        
    start_hashkey=bp->hashkey^hashkey_at_move[GOLD]^hashkey_at_move[SILVER];
    number_of_steps=4;
    depth_limit=8;
    for (i=0; steps<number_of_steps; i++)
    {
        start_time=clock();
        sprintf(message,"\nSearching position:\n");
        BOARD_Message();
        BOARD_Print_Position(bp);
        sprintf(message,"\n         Depth  Value     Nodes     Evals   Time  PV\n");
        BOARD_Message();
        total_nodes=0;
        total_evals=0;
        elapsed_time=0;
        for (j=1; j<=depth_limit; j++)
        {
            depth=0;
            max_depth=0;
            sum_eval_depths=0;
            nodes=0;
            evals=0;
            value=Nega_Max(bp,-999999,999999,j);
            if (value==500000-bp->move*4-bp->steps) // We've already won in the root position, so need just to pass
            {
                pv[0][0].pass=TRUE;
                pv[0][0].steps=4-bp->steps;
                pv_length[0]=1;
                hash_hit[0]=-1;
            }
            elapsed_time=clock()-start_time;
            Show_Search_Info(bp,TRUE,j,value);
            total_nodes+=nodes;
            total_evals+=evals;
            BOARD_Copy_Move(&pv[0][0],&ml[i]);
        }
        steps+=ml[i].steps;
        BOARD_Do_Move(bp,&ml[i]);
    }
    i--;
    for (; i>=0; i--)
    {
        BOARD_Undo_Move(bp,&ml[i]);
    }
    sprintf(message,"\n");
    BOARD_Message();
    return value;
}

int Nega_Max(board_t *bp, int lower_bound, int upper_bound, int steps)
{
    int value;
    int number_of_moves;
    move_t move_list[MAX_NUMBER_MOVES];
    move_t *current_move;
    int i, j;
    int cutoff=FALSE;
    int original_lower_bound, original_upper_bound;
    int terminal_node=FALSE;
    hash_entry_t *hp;
    move_t hash_move;
        
    nodes++;
    original_lower_bound=lower_bound; // need these when writing position to hashtable to know if we're writing a lower, upper or exact value
    original_upper_bound=upper_bound;
    hp=HASH_Get_Entry(bp);
    if (hp!=NULL && depth>0) // Check if position is in hash table.
    {
        // Check if we can use value from hash table.
		if (hp->depth>=steps)
		{
			switch (hp->type)
			{
				case EXACT_VALUE :
                    pv_length[depth]=0;
                    hash_hit[depth]=0;
					return hp->value;
					break;
				case LOWER_BOUND :
					if (hp->value>=upper_bound)
					{
                        pv_length[depth]=0;
                        hash_hit[depth]=0;
						return hp->value;
					}
					break;
				case UPPER_BOUND :
					if (hp->value<=lower_bound)
					{
                        pv_length[depth]=0;
                        hash_hit[depth]=0;
						return hp->value;
					}
					break;
			}
		}			
    }
    // Check to see if either player has won the game.
    for (i=A8; i<=H8; i+=EAST)
    {
        if (BOARD(i)==(GOLD | RABBIT_PIECE))
        {
            if (bp->at_move==GOLD)
            {
                lower_bound=500000-bp->move*4-bp->steps;
            } else
            {
                lower_bound=-500000+bp->move*4+bp->steps;
            }
            cutoff=TRUE;
        }
    }
    for (i=A1; i<=H1; i+=EAST)
    {
        if (BOARD(i)==(SILVER | RABBIT_PIECE))
        {
            if (bp->at_move==GOLD)
            {
                lower_bound=-500000+bp->move*4+bp->steps;
            } else
            {
                lower_bound=500000-bp->move*4-bp->steps;
            }
            cutoff=TRUE;
        }
    }
    if (cutoff) // somebody has won the game... no need to search further
    {
        pv_length[depth]=0;
        hash_hit[depth]=-1;
        if (depth>max_depth) max_depth=depth;
        terminal_node=TRUE;
    } else
    {
        if (steps<=0) // we've reached the bottom of the search tree, so call Evaluation function
        {
            pv_length[depth]=0;
            hash_hit[depth]=-1;
            evals++;
            if (depth>max_depth) max_depth=depth;
            sum_eval_depths+=depth;
            lower_bound=EVAL_Eval(bp,FALSE);
            terminal_node=TRUE;
        } else
        {
            number_of_moves=BOARD_Generate_Moves(bp,move_list); // Generate proper movelist                            
            for (i=0; !cutoff && i<number_of_moves; i++)
            {
                current_move=&move_list[i];
                BOARD_Do_Move(bp,current_move);
                depth+=current_move->steps;
                steps-=current_move->steps;
                if (bp->hashkey!=start_hashkey) // We're not returning to the start position
                {
                    if (bp->steps==0) // different player at move
                    {
                        value=-Nega_Max(bp,-upper_bound,-lower_bound,steps);
                    } else // same player at move
                    {
                        value=Nega_Max(bp,lower_bound,upper_bound,steps);
                    }
                    BOARD_Undo_Move(bp,current_move);
                    depth-=current_move->steps;
                    steps+=current_move->steps;
                    if (value>lower_bound)
                    {
                        lower_bound=value;
                        BOARD_Copy_Move(current_move,&hash_move);
                        if (lower_bound>=upper_bound) // cutoff, so we want to exit right away
                        {
                            cutoff=TRUE;
                        } else
                        { // Maintain partial PV
                            pv_length[depth]=pv_length[depth+current_move->steps]+current_move->steps;
                            if (hash_hit[depth+current_move->steps]==-1)
                            {
                                hash_hit[depth]=-1;
                            } else
                            {
                                hash_hit[depth]=hash_hit[depth+current_move->steps]+current_move->steps;
                            }
                            for (j=depth+current_move->steps; j<depth+pv_length[depth]; j+=pv[depth][j].steps)
                            {
                                BOARD_Copy_Move(&pv[depth+current_move->steps][j],&pv[depth][j]);
                            }
                            BOARD_Copy_Move(current_move,&pv[depth][depth]);
                        }
                        if (depth==0)
                        {
                            Show_Search_Info(bp,FALSE,steps,value);
                        }
                    }
                } else
                {
                    BOARD_Undo_Move(bp,current_move);
                    depth-=current_move->steps;
                    steps+=current_move->steps;
                }
            }
            if (number_of_moves==0) // win by immobilization
            {
                lower_bound=-500000+bp->move*4+bp->steps;
                terminal_node=TRUE;
                pv_length[depth]=0;
                hash_hit[depth]=-1;
                if (depth>max_depth) max_depth=depth;
            }
        }
    }
    // Write to hashtable
    if (terminal_node) // Terminal node, so we clearly have an exact value
    {
        if (steps>0)
        {
            HASH_Put_Entry(bp->hashkey,lower_bound,steps,EXACT_VALUE,NULL);
        } else
        {
            HASH_Put_Entry(bp->hashkey,lower_bound,0,EXACT_VALUE,NULL);
        }
    } else
    {
        if (lower_bound==original_lower_bound) // Fail-low, so we have an upper bound for the real value of position
        {
            HASH_Put_Entry(bp->hashkey,original_lower_bound,steps,UPPER_BOUND,NULL);
        } else if (lower_bound>=original_upper_bound) // Fail-high, so we have a lower bound for the real value of position
        {
            HASH_Put_Entry(bp->hashkey,original_upper_bound,steps,LOWER_BOUND,&hash_move);
        } else // Value is between original lower and upper bounds, so we have an exact value for the position
        {
            HASH_Put_Entry(bp->hashkey,lower_bound,steps,EXACT_VALUE,&hash_move);
        }
    }
    return lower_bound;
}
