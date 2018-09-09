#include "CAbstractHuffPipe.h"

/**
    Initialize counters for each symbol (ASCII chars).
    Initialize data counter to 0.
 */
OSErr CAbstractHuffPipe::Open() {
    short i;
    for (i = 0; i < NUMSYMBOLS; i++) {
        symbCounters[i] = 1;
    }
    dataCount = 0;
    return CAbstractPipe::Open();
}

/**
    Shell sorts the first NUMSYMBOLS nodes.
 */
void CAbstractHuffPipe::SortNodes(HuffTreeNode **table) {
    short h;
    HuffTreeNode **ip, **jp, *vp;
    HuffTreeNode **lastP;
    HuffTreeNode **hp;

    h = 1;
    lastP = table + NUMSYMBOLS;

    do {
        h += h + h + 1;
    } while (h <= NUMSYMBOLS);

    do {
        h /= 3;
        hp = table + h;
        for (ip = hp; ip < lastP; ip++) {
            vp = *ip;
            jp = ip;
            while (jp[-h]->count > vp->count) {
                *jp = jp[-h];
                jp -= h;
                if (jp < hp) {
                    break;
                }
            }
            *jp = vp;
        }
    } while (h != 1);
}

/**
    Builds a Huffman tree using the given symbol distribution.
    The root node of the tree is returned.
 */
HuffTreeNode *CAbstractHuffPipe::BuildTree() {
    HuffTreeNode *subTrees[NUMSYMBOLS + 1];
    HuffTreeNode **treeList;
    HuffTreeNode *freshNode;
    HuffTreeNode markerNode;
    short i;

    // Place symbols into the first NUMSYMBOLS places in the tree array.
    for (i = 0; i < NUMSYMBOLS; i++) {
        nodes[i].count = symbCounters[i];
        nodes[i].left = -1;
        nodes[i].right = i;
    }

    // Put pointers to the tree array into the subTrees array.
    for (i = 0; i < NUMSYMBOLS; i++) {
        subTrees[i] = &(nodes[i]);
    }

    // Sort pointers in the subTree array so that the symbol with the lowest count is first, etc.
    SortNodes(subTrees);

    // freshNode points to a free tree node:
    freshNode = &(nodes[NUMSYMBOLS]);

    // Pointer to current list of subtrees:
    treeList = subTrees;

    // Create a marker node to stop insertion sorts:
    markerNode.count = dataCount + NUMSYMBOLS + 1;
    subTrees[NUMSYMBOLS] = &markerNode;

    i = 1;
    while ((*treeList)->count == 0) {
        i++;
        treeList++;
    }

    // Reduce tree into a root node. Requires NUMSYMBOLS-1 steps:
    for (; i < NUMSYMBOLS; i++) {
        HuffTreeNode *aNode, *bNode;
        HuffTreeNode **sorter;
        long count;

        // Combine the two subtrees with the lowest counts:
        aNode = treeList[0];
        bNode = treeList[1];
        treeList++;

        count = aNode->count + bNode->count;

        freshNode->count = count;
        freshNode->left = aNode - nodes;
        freshNode->right = bNode - nodes;

        // Sort the new subtree into the list of subtrees:
        sorter = treeList + 1;

        // >= is used to keep order in the equal distribution case. > could be used as well.
        while (count >= (*sorter)->count) {
            sorter[-1] = sorter[0];
            sorter++;
        }

        sorter[-1] = freshNode;

        freshNode++;
    }

    return treeList[0];
}

/**
    This abstract class doesn't pass any data forward. This method was used to test the tree
    creation code.
 */
OSErr CAbstractHuffPipe::PipeData(Ptr dataPtr, long len) {
    dataCount += len;
    while (len--) {
        symbCounters[*(unsigned char *)(dataPtr++)]++;
    }
    return noErr;
}
