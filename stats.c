#include "stats.h"
#include "settings.h"

typedef struct
{
    double count;
    double min;
    double max;
    double sum;
} TStats;


ListNode *Stats=NULL;
void StatsInsert(const char *Title, double timediff)
{
    ListNode *Node;
    TStats *Item;

    if (! Stats) Stats=ListCreate();
    Node=ListFindNamedItem(Stats, Title);
    if (Node) Item=(TStats *) Node->Item;
    else
    {
        Item=(TStats *) calloc(1, sizeof(TStats));
        Item->min=-1;
        ListAddNamedItem(Stats, Title, Item);
    }

    Item->count++;
    if ((Item->min == -1) || (timediff < Item->min)) Item->min=timediff;
    if (timediff > Item->max) Item->max=timediff;
    Item->sum += timediff;
}


void StatsDisplay()
{
    TStats *Item;
    ListNode *Node;

    Node=ListGetNext(Stats);
    while (Node)
    {
        Item=(TStats *) Node->Item;
	if (Settings.Flags & FLAG_NANOSECS) printf("%20s ITEMS: %8.0f MIN: %14.6f  MAX: %14.6f  AVG %14.6f\n", Node->Tag, Item->count, Item->min, Item->max, Item->sum / Item->count);
        else printf("%20s ITEMS: %8.0f MIN: %10.3f  MAX: %10.3f  AVG %10.3f\n", Node->Tag, Item->count, Item->min, Item->max, Item->sum / Item->count);
        Node=ListGetNext(Node);
    }

}


