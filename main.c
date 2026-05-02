#include <stdio.h>
#include <raylib.h>
#include <raymath.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#define WW 800
#define WH 480 
#define MAG 100 
#define ROW_MAX_RECS 50 
#define BG 0x800090ff
#define FG 0x008090ff
#define HL 0x800000ff
#define DN 0x008000ff
#define IDX_TXT_H 10
#define TITLE_TXT_H 40 
#define GUI_TXT_SZ 30

typedef enum {
    UNDEF,
    BUBBLE,
    INSERT,
    QUICK,
} SortType;
static const char *options = "Select Method;Bubble;Insertion;Quick";
static void GetSelection(SortType *st);

static Color GetColor32(uint32_t iclr);
static void GetRandHeightRects(Rectangle rects[]);
static int SortRectsStep(SortType sr, Rectangle rects[], int *idxa, int *idxb);
static int BubbleSortStep(Rectangle rects[], int *idxa, int *idxb);
static int InsertionSortStep(Rectangle rects[], int *idxa, int *idxb);
static int QuickSortStep(Rectangle rects[], int *idxa, int *idxb);
static Vector2 GetTitle(char *buf, SortType);
static Vector2 GetResultText(char *buf, double end, uint32_t steps);
static Color GetRectColor(int sort, int idx, int sort_idx_a, int sort_idx_b);
static void SwapRects(Rectangle rects[], int a, int b);

int main(void)
{
    double runtime = 0;
    srandom(time(NULL));
    Rectangle rects[MAG] = {0};
    GetRandHeightRects(rects);
    SetTargetFPS(25);
    InitWindow(WW, WH, "VisualSort");
    char title[16];
    char result[64];
    char buf[16];
    char scshot_name[32];
    time_t scshot;
    struct tm *scshot_s;
    int idxTxtYPos = 0, swap = 0, sort_idx_a, sort_idx_b;
    uint32_t steps = 0;
    Vector2 titlePos, resultPos;
    Color rectColor; 
    SortType stprev = UNDEF;
    SortType st = UNDEF;
    titlePos = GetTitle(title, st);
    GuiSetStyle(DEFAULT, TEXT_SIZE, GUI_TXT_SZ);
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0x000000ff);

    bool done, pause = false;

    while(!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(GetColor32(BG));
        DrawText(title, titlePos.x, titlePos.y, TITLE_TXT_H, WHITE);

        if (st != UNDEF)
        {
            for (int i = 0; i < MAG; ++i)
            {
                sprintf(buf, "%d", (int)rects[i].height);
                rectColor = GetRectColor(swap, i, sort_idx_a, sort_idx_b);
                DrawRectangleRec(rects[i], rectColor);
                Vector2 topLeft = (Vector2) { rects[i].x, rects[i].y };
                Vector2 btmLeft = (Vector2) { rects[i].x, rects[i].y + rects[i].height };
                Vector2 topRight = (Vector2) { rects[i].x + rects[i].width, rects[i].y };
                DrawLineV(topLeft, topRight, BLACK);
                DrawLineV(topLeft, btmLeft, BLACK);
                idxTxtYPos = rects[i].height < IDX_TXT_H ? (rects[i].y + rects[i].height) - IDX_TXT_H : rects[i].y;
                DrawText(buf, rects[i].x, idxTxtYPos, IDX_TXT_H, WHITE);
            }
            if (done)
            {
                DrawText(result, resultPos.x, resultPos.y, TITLE_TXT_H, WHITE);
                if (GuiButton((Rectangle){
                        resultPos.x, 
                        resultPos.y + GUI_TXT_SZ + 4, 
                        MeasureText("RESTART", GUI_TXT_SZ) + 4, 
                        GUI_TXT_SZ + 6 
                    }, 
                    "RESTART"))
                {
                    st = UNDEF;
                }
            }

        }
        else
        {
            stprev = st;
            GetSelection(&st);
        }
                    
        EndDrawing();
        
        if (IsKeyPressed(KEY_S))
        {
            scshot = time(NULL);
            scshot_s = gmtime(&scshot);
            strftime(scshot_name, sizeof(scshot_name), "scshot_%Y%m%d%H%M%S.png", scshot_s);
            TakeScreenshot(scshot_name);
            printf("Screenshot %s taken\n", scshot_name);
        }

        if (IsKeyPressed(KEY_P))
        {
            pause = !pause;
        }


        if (stprev != st) 
        {
            if (st != UNDEF)
            {
                // reset with chosen sorting method
                GetRandHeightRects(rects);
                done = false;
                swap = 0;
            }
            titlePos = GetTitle(title, st);
        }

        if (st != UNDEF && !pause)
        {
            if (swap != 2)
            {
                clock_t begin_rt = clock();
                if ((swap = SortRectsStep(st, rects, &sort_idx_a, &sort_idx_b)) < 0) break;
                runtime += (double)(clock() - begin_rt) / CLOCKS_PER_SEC;
                steps++;
            }
            else if (!done)
            {
                resultPos = GetResultText(result, runtime, steps);
                done = true;
            }
        }
        stprev = st;
    }
    CloseWindow();
    return 0;
}

static void GetRandHeightRects(Rectangle rects[])
{
    int rows = ceilf(MAG / (float)ROW_MAX_RECS); // 96 / 48 = 2
    int rowH = WH / rows;
    float recW = WW / (float)ROW_MAX_RECS; 
    
    for (int i = 0; i < MAG; ++i)
    {
        int randHeight = random() % WH / rows;
        int rowIdx = i / ROW_MAX_RECS + 1;
        rects[i].x = (int)(i * recW) % WW;
        rects[i].y = rowH * rowIdx - randHeight;
        rects[i].width = recW;
        rects[i].height = randHeight;
    }
}

static Color GetColor32(uint32_t iclr)
{
    return (Color) {
        iclr >> 24 & 0xff, 
        iclr >> 16 & 0xff,
        iclr >> 8 & 0xff,
        iclr & 0xff
    };
}

static int SortRectsStep(SortType sr, Rectangle rects[], int *idxa, int *idxb)
{
    switch(sr)
    {
        case INSERT:
            return InsertionSortStep(rects, idxa, idxb);
            break;
        case BUBBLE:
            return BubbleSortStep(rects, idxa, idxb);
            break;
        case QUICK:
            return QuickSortStep(rects, idxa, idxb);
            break;
        default:
            return -1;
            break;
    }
}

static int BubbleSortStep(Rectangle rects[], int *idxa, int *idxb)
{
    int rc = 0;
    static int a = 0, sorted = MAG - 1;
    if (rects[a].height > rects[a + 1].height)
    {
        SwapRects(rects, a, a + 1);
        rc = 1;
    }
    *idxa = a;
    *idxb = a+1;
    if (a == sorted - 1)
    {
        a = 0;
        sorted--;
    }
    else
    {
        a++;
    }
    if (sorted == 1)
    {
        a = 0; sorted = MAG - 1;
        return 2;
    }
    return rc;
}

static int InsertionSortStep(Rectangle rects[], int *idxa, int *idxb)
{
    static int rc = -1, a = 1, b = 0;
    
    if (a == MAG - 1 && (b < 0 || rc == 0)) 
    {
        a = 1; b = 0; rc = -1;
        return 2;
    }
    else if (b < 0 || rc == 0) 
    {
        a++;
        b = a - 1;
    }


    if (rects[b+1].height < rects[b].height)
    {
        SwapRects(rects, b+1, b);
        *idxa = b;
        *idxb = b+1;
        rc = 1;
    } 
    else
    {
        rc = 0;
    }

    b--;

    return rc;
}

static int CompareInt(const void *int1, const void *int2)
{
    return *(const int *)int1 - *(const int *)int2;
}

static int QuickSortStep(Rectangle rects[], int *idxa, int *idxb)
{
    static int i = 0, j = MAG - 1, k = 0, pVal = -2;
    static int p[3] = { -1, -1, -1 };
    int rc = 0;

    if (pVal < 0)
    {
        // get the median of 3
        p[0] = (rand() % (j - i + 1)) + i;
        p[1] = (rand() % (j - i + 1)) + i;
        p[2] = (rand() % (j - i + 1)) + i;
        qsort(p, 3, sizeof(int), CompareInt);
        pVal = rects[p[1]].height;
        i--; j++;
    }

    do
    {
        j--;
    } while (rects[j].height > pVal);
    
    do
    {
        i++;
    } while ( rects[i].height < pVal );

    if (i >= j)
    {
        if (j > k)
        {
            i = k;
        }
        else if (k >= MAG - 1)
        {
            i = 0; j = MAG - 1; k = i; pVal = -1; // RESET for next run
            rc = 2; // DONE!
        }
        else
        {
            i = k + 1;
            k = i;
            j = MAG - 1;
        }
        pVal = -1;
    }
    else
    {
        SwapRects(rects, i, j);
        *idxa = i;
        *idxb = j;
        rc = 1;
    }

    return rc;
}

static Vector2 GetTitle(char *buf, SortType st)
{
    switch(st)
    {
        case BUBBLE:
            sprintf(buf, "%s", "BUBBLE");
            break;
        case INSERT:
            sprintf(buf, "%s", "INSERTION");
            break;
        case QUICK:
            sprintf(buf, "%s", "QUICK");
            break;
        default:
            sprintf(buf, "%s", "SORT VISUALIZER");
            break;
    }

    return (Vector2) { 
        .x = WW / 2 - MeasureText(buf, TITLE_TXT_H) / 2,
        .y = 2 
    };
}

static void GetSelection(SortType *st)
{
    static int active = 0;
    static bool edit = false;
    bool tmpedit = false;
    Rectangle bounds = (Rectangle){WW/3, TITLE_TXT_H + 4,WW/3,WH/8}; 
    tmpedit = GuiDropdownBox(bounds, options, &active, edit);
    if (tmpedit) edit = true; 
    if (active > 0)
    {
        *st = active;
        active = 0;
        edit = false;
    }
}

static Color GetRectColor(int sort, int idx, int sort_idx_a, int sort_idx_b)
{   
    if (sort == 1 && (idx == sort_idx_a || idx == sort_idx_b)) 
    {
        return GetColor32(HL);
    }
    else if (sort == 2)
    {
        return GetColor32(DN);
    }
    else
    {
        return GetColor32(FG);
    }
}
static void SwapRects(Rectangle rects[], int a, int b)
{
    int rows = ceilf(MAG / (float)ROW_MAX_RECS); // 96 / 48 = 2
    int rowH = WH / rows;
    int rowIdxA = a / ROW_MAX_RECS + 1;
    int rowIdxB = b / ROW_MAX_RECS + 1;
    float htmp = rects[a].height;
    rects[a].y = rowIdxA * rowH - rects[b].height;
    rects[a].height = rects[b].height;
    rects[b].y = rowIdxB * rowH - htmp;
    rects[b].height = htmp;
}

static Vector2 GetResultText(char *buf, double end, uint32_t steps)
{
    sprintf(buf, "time %.05fs, %u steps", end, steps);
    return (Vector2) { 
        WW / 2 - MeasureText(buf, TITLE_TXT_H) / 2,
        WH / 2 - TITLE_TXT_H / 2
    };
}

