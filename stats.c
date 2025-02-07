#include "stats.h"
#include "settings.h"
#include "common.h"

typedef struct
{
    double count;
    double min;
    double max;
    double sum;
    struct timeval start;
    struct timeval end;
} TStats;


ListNode *Stats=NULL;


TStats *StatsCreate(const char *Title, struct timeval *When)
{
    TStats *Item;

    Item=(TStats *) calloc(1, sizeof(TStats));
    Item->min=-1;

    Item->start.tv_sec=When->tv_sec;
    Item->start.tv_usec=When->tv_usec;

    ListAddNamedItem(Stats, Title, Item);

    return(Item);
}


void StatsInsert(const char *Title, struct timeval *When, double timediff)
{
    ListNode *Node;
    TStats *Item;

    if (! Stats) Stats=ListCreate();
    Node=ListFindNamedItem(Stats, Title);
    if (Node)
    {
        Item=(TStats *) Node->Item;
        //only set the minimum if this isn't the first item, it'll always be 0.00 for the first
        if ((Item->min == -1) || (timediff < Item->min)) Item->min=timediff;
    }
    else Item=StatsCreate(Title, When);

    Item->end.tv_sec=When->tv_sec;
    Item->end.tv_usec=When->tv_usec;

    Item->count++;
    if (timediff > Item->max) Item->max=timediff;
    Item->sum += timediff;
}


void StatsDisplay()
{
    TStats *Item;
    ListNode *Node;
    double total;

    Node=ListGetNext(Stats);
    while (Node)
    {
        Item=(TStats *) Node->Item;
        total=CalcTimediff( &(Item->start), &(Item->end));

        if (Settings.Flags & FLAG_NANOSECS) printf("%20s ITEMS: %6.0f  MIN: %14.6f  MAX: %14.6f  AVG: %14.6f  TOTAL: %14.6f\n", Node->Tag, Item->count, Item->min, Item->max, Item->sum / Item->count, total);
        else printf("%20s ITEMS: %6.0f  MIN: %8.3f  MAX: %8.3f  AVG: %8.3f  TOTAL: %8.3f\n", Node->Tag, Item->count, Item->min, Item->max, Item->sum / Item->count, total);
        Node=ListGetNext(Node);
    }

}


