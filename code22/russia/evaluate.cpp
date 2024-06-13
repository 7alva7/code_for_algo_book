#include "stdafx.h"
#include "evaluate.h"

int sum_n[] = { 0, 1, 3, 6, 10, 15, 21, 28, 36, 45, 55, 66, 78, 91, 105, 120, 136, 153, 171, 190, 210 };

bool IsFullRowStatus(RUSSIA_GAME *game, int row)
{
    for(int i = 1; i <= GAME_COL; i++)
    {
        if(game->board[row][i] == 0)
            return false;
    }

    return true;
}

int GetLandingHeight(RUSSIA_GAME *game, B_SHAPE *bs, int row, int col)
{
    return GAME_ROW - (row + bs->height - 2);
}

int GetErodedPieceCellsMetric(RUSSIA_GAME *game, B_SHAPE *bs, int row, int col)
{
    int erodedRow = 0;
    int erodedShape = 0;
    int i = game->top_row + 1;
    while(i <= GAME_ROW)
    {
        if(IsFullRowStatus(game, i))
        {
            erodedRow++;
            if((i >= row) && (i <= (row + bs->height)))
            {
                int sline = i - row;
                for(int j = 0; j < bs->width; j++)
                {
                    if(bs->shape[sline][j] != 0)
                    {
                        erodedShape++;
                    }
                }
            }
        }
        i++;
    }

    return (erodedRow * erodedShape);
}

int GetBoardRowTransitions(RUSSIA_GAME *game, B_SHAPE *bs, int row, int col)
{
    //���ұ߽����仯�������������α仯
    int transitions = game->top_row * 2;
    for(int i = game->top_row + 1; i <= GAME_ROW; i++)
    {
        int lastCell = S_B;// ��߽����仯�ж�
        for(int j = 1; j < BOARD_COL; j++) //�ұ߽����仯���ж�
        {
            // ����һ����Ԫ����ͬ
            if (!(lastCell * game->board[i][j]) && (lastCell != game->board[i][j]))
            {
                lastCell = game->board[i][j];
                transitions++;
            }
        }
        //if (game->board[i][BOARD_COL] == 0)
        //    transitions++;
    }

    return transitions;
}

int GetBoardColTransitions(RUSSIA_GAME *game, B_SHAPE *bs, int row, int col)
{
    int transitions = 0;
    for(int j = 1; j <= GAME_COL; j++)
    {
        int lastCell = S_B;// �ϱ߽����仯�ж�
        for(int i = 1; i < BOARD_ROW; i++) //�±߽����仯���ж�
        {
            // ����һ����Ԫ����ͬ
            if (!(lastCell * game->board[i][j]) && (lastCell != game->board[i][j]))
            {
                lastCell = game->board[i][j];
                transitions++;
            }
        }
        
        //if (game->board[GAME_ROW][j] == 0)
        //    transitions++;
    }

    return transitions;
}

#if 0
//�����Ŀ�Cell��Ϊһ��hole
int GetBoardBuriedHoles(RUSSIA_GAME *game, B_SHAPE *bs, int row, int col)
{
    int holes = 0;
    for(int j = 0; j < GAME_COL; j++)
    {
        int i = game->top_row;
        while((game->board[i + 1][j + 1] == 0) && (i < GAME_ROW))
            i++;
        while(i < GAME_ROW)
        {
            if(game->board[i + 1][j + 1] == 0)
            {
                holes++;
                /*�����Ŀ�Cell��Ϊһ��hole*/
                while((game->board[i + 1][j + 1] == 0) && (i < GAME_ROW))
                    i++;
            }
            else
            {
                i++;
            }
        }
    }

    return holes;
}
#endif
#if 1
int GetBoardBuriedHoles(RUSSIA_GAME *game, B_SHAPE *bs, int row, int col)
{
    int holes = 0;
    for(int j = 1; j <= GAME_COL; j++)
    {
        int i = game->top_row;
        bool holeStart = false; 
        while (i <= GAME_ROW)
        {
            if (!holeStart)
            {
                if (game->board[i][j] != 0)
                    holeStart = true;
            }
            else
            {
                if (game->board[i][j] == 0)
                    holes++;
            }

            i++;
        }
    }

    return holes;
}
#endif
int GetBoardWells(RUSSIA_GAME *game, B_SHAPE *bs, int row, int col)
{
    int sum = 0;
    for(int j = 1; j <= GAME_COL; j++)
    {
        int wells = 0;
        bool wallStart = false;
        for(int i = game->top_row + 1; i <= GAME_ROW; i++)
        {
            if(game->board[i][j] == 0)
            {
                if(wallStart)
                    wells++;
                else
                {
                    if((game->board[i][j - 1] != 0) && (game->board[i][j + 1] != 0))
                    {
                        wallStart = true;
                        wells = 1;
                    }
                }
            }
            else
                break;
        }
        sum += sum_n[wells];
    }

    return sum;
}

//Pierre Dellacherie�㷨���ۺ���
/*
rating = (-1.0) * landingHeight + ( 1.0) * erodedPieceCellsMetric
         + (-1.0) * boardRowTransitions + (-1.0) * boardColTransitions
         + (-4.0) * boardBuriedHoles + (-1.0) * boardWells;

El-Tetris �ṩ��ϵ��
    1   -4.500158825082766 
    2   3.4181268101392694 
    3   -3.2178882868487753 
    4   -9.348695305445199 
    5   -7.899265427351652 
    6   -3.3855972247263626 

*/
int EvaluateFunction(RUSSIA_GAME *game, B_SHAPE *bs, int row, int col)
{
    int evalue = 0;

    int lh = GetLandingHeight(game, bs, row, col);
    int epcm = GetErodedPieceCellsMetric(game, bs, row, col);
    int brt = GetBoardRowTransitions(game, bs, row, col);
    int bct = GetBoardColTransitions(game, bs, row, col);
    int bbh = GetBoardBuriedHoles(game, bs, row, col);
    int bw = GetBoardWells(game, bs, row, col);

    //evalue = (-1) * lh + epcm - (4 * bbh) - (1 * bw);
    //evalue = (-1) * lh + epcm - (0.5 * brt) - (0.5 * bct) - (6 * bbh) - (1 * bw);
    //evalue = (-1) * lh + (1) * epcm + (-1 * brt) + (-1 * bct) + (-4 * bbh) + (-1 * bw);
    //evalue = (-1) * lh + epcm - brt - bct - (4 * bbh) - bw;
    evalue = -45 * lh + 34 * epcm -32 * brt - 98 * bct - 79 * bbh - 34 * bw;

    return evalue;
}
/*
�������������:priority = 100 * ����ˮƽƽ�Ƹ����� + 10 + ������ת����;

�����������Ҳ�:priority = 100 * ����ˮƽƽ�Ƹ����� + ������ת����;
*/
int PrioritySelection(RUSSIA_GAME *game, B_SHAPE* bs, int row, int col)
{
    int priority = 0;

    if(col <= (GAME_COL / 2))
    {
        priority = 100 * ((GAME_COL / 2) - col) + 10 + bs->r_index;
    }
    else
    {
        priority = 100 * (col - (GAME_COL / 2)) + bs->r_index;
    }

    return priority;
}
