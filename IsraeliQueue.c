#include "IsraeliQueue.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

typedef struct Node_t* Node;

struct Node_t {
    void* m_data;
    int m_friendsCalledOver;
    int m_rivalsBlocked;
    Node m_next;
};

struct IsraeliQueue_t {
    Node m_list;
    FriendshipFunction* m_friendships;
    int m_friendshipsLength;
    ComparisonFunction m_compare;
    int m_friendshipThreshold;
    int m_rivalryThreshold;
};

typedef enum FriendStatus {
    NEUTRAL = 0,
    FRIEND = 1,
    RIVAL = -1,
} FriendStatus;


// === Utility Functions ===

// Copy the contents of the data to a new buffer of the given size.
void* copyToMallocResize(void* data, int dataSize, int outSize) {
    void* ret = calloc(outSize, 1);

    // Return early if malloc failed.
    if (!ret) {
        return NULL;
    }

    memcpy(ret, data, dataSize > outSize ? outSize : dataSize);

    return ret;
}

// Copy the contents of a buffer to a new buffer of the same size.
void* copyToMalloc(void* data, int size) {
    return copyToMallocResize(data, size, size);
}

// Get the size of a null terminated array of FriendshipFunctions.
int sizeOfFriendshipArray(FriendshipFunction* arr) {
    int size = 0;
    for (int i = 0; arr[i] != NULL; i++) {
        size++;
    }
    return size;
}

FriendStatus getFriendshipStatus(IsraeliQueue q, void* data1, void* data2) {
    // Iterate over the friendship functions and sum their results.
    // Exit early if one of the functions returns a value that is friendly enough.
    int friendshipSum = 0;
    for (int i = 0; i < q->m_friendshipsLength; i++) {
        int friendshipNumber = q->m_friendships[i](data1, data2);
        friendshipSum += friendshipNumber;

        // Exit early if the friendship is friendly enough.
        if (friendshipSum > q->m_friendshipThreshold) {
            return FRIEND;
        }
    }

    // Check if the objects are enemies.
    float friendshipAverage = ((float)friendshipSum) / q->m_friendshipsLength;
    if (friendshipAverage < q->m_rivalryThreshold) {
        return RIVAL;
    }

    return NEUTRAL;
}


// === Node Functions ===

// Initialize a new node in place.
void NodeInit(Node node, void* data, Node next) {
    node->m_data = data;
    node->m_friendsCalledOver = 0;
    node->m_rivalsBlocked = 0;
    node->m_next = next;
}

// Create a new node and malloc it.
Node NodeCreate(void* data, Node next) {
    Node ret = (Node)malloc(sizeof(struct Node_t));
    NodeInit(ret, data, next);
    return ret;
}


// Returns the first none-blocked friend. If all friends are blocked, returns
// the first rival that is blocking. If no friends were found, returns the last
// node in the list.
// According to those cases, sets the outStatus to the appropriate value.
Node* findFriendNotBlocked(IsraeliQueue q, void* data, Node stop, FriendStatus* outStatus) {
    // Iterativly, find the first friend, and if it is blocked, start over.
    Node* friend = NULL;
    Node* rival = NULL;
    Node* last = &q->m_list;
    for (Node* curr = &q->m_list; *curr != NULL && *curr != stop; curr = &(*curr)->m_next) {
        // According to the friendship status, update the friend and rival.
        FriendStatus status = getFriendshipStatus(q, data, (*curr)->m_data);
        // TODO: Allow a friend that is full but contains the data to be returned.
        if (status == FRIEND && (*curr)->m_friendsCalledOver < FRIEND_QUOTA && !friend) {
            friend = curr;
        } else if (status == RIVAL && (*curr)->m_rivalsBlocked < RIVAL_QUOTA && friend) {
            friend = NULL;
            rival = curr;
        }
        // Update the last node.
        last = curr;
    }

    *outStatus = friend ? FRIEND : rival ? RIVAL : NEUTRAL;
    return friend ? friend : rival ? rival : last;
}

// Insert a node after the given node. Updates the friends and rivals
// lists of insertAfter according to the given friendship status.
void NodeInsertAfter(Node* insertAfter, Node* toInsertPtr, FriendStatus status) {
    assert(insertAfter);
    assert(toInsertPtr);
    assert((*toInsertPtr)->m_next);

    if (*insertAfter == NULL) {
        *insertAfter = *toInsertPtr;
        return;
    }

    /* TODO:
    // Regular linked list insertion.
    Node oldNext = (*toInsertPtr)->m_next;
    (*toInsertPtr)->m_next = (*insertAfter)->m_next;
    (*insertAfter)->m_next = *toInsertPtr;
    *toInsertPtr = oldNext;
    */
    (*toInsertPtr)->m_next = (*insertAfter)->m_next;
    (*insertAfter)->m_next = *toInsertPtr;
    
    // Update the friends and rivals counters.
    if (status == FRIEND) {
        (*insertAfter)->m_friendsCalledOver++;
    } else if (status == RIVAL) {
        (*insertAfter)->m_rivalsBlocked++;
    }
}


// === Implementation ===

IsraeliQueue IsraeliQueueCreate(FriendshipFunction* friendships, ComparisonFunction compare, int friendshipThreshold, int rivalryThreshold) {
    IsraeliQueue ret = (IsraeliQueue)malloc(sizeof(struct IsraeliQueue_t));

    // Return early if malloc failed.
    if (!ret) {
        return NULL;
    }

    // Copy the friendship functions.
    int functions = sizeOfFriendshipArray(friendships);
    FriendshipFunction* friendshipsCopied = (FriendshipFunction*)copyToMalloc(friendships, sizeof(FriendshipFunction) * (functions + 1));

    ret->m_list = NULL;
    ret->m_friendships = friendshipsCopied;
    ret->m_friendshipsLength = functions;
    ret->m_compare = compare;
    ret->m_friendshipThreshold = friendshipThreshold;
    ret->m_rivalryThreshold = rivalryThreshold;
    printf("Creating queue\n");
    return ret;
}

IsraeliQueue IsraeliQueueClone(IsraeliQueue q) {
    IsraeliQueue out = IsraeliQueueCreate(q->m_friendships, q->m_compare, q->m_friendshipThreshold, q->m_rivalryThreshold);
    // Clone over the data.
    Node* outNode = &out->m_list;
    for (Node inNode = q->m_list; inNode != NULL; inNode = inNode->m_next) {
        *outNode = NodeCreate(inNode->m_data, NULL);
        (*outNode)->m_friendsCalledOver = inNode->m_friendsCalledOver;
        (*outNode)->m_rivalsBlocked = inNode->m_rivalsBlocked;
        outNode = &(*outNode)->m_next;
    }
    printf("Cloning queue\n");
    return out;
}

void NodeDestroy(Node n) {
    if (!n) {
        return;
    }

    NodeDestroy(n->m_next);
    free(n);
}

void IsraeliQueueDestroy(IsraeliQueue q) {
    // Exit early if the queue is already NULL.
    if (!q) {
        return;
    }

    NodeDestroy(q->m_list);

    free(q->m_friendships);
    printf("freed que\n");
    free(q);
}

IsraeliQueueError IsraeliQueueEnqueue(IsraeliQueue q, void* data) {
    FriendStatus status = 0;
    Node* insertAfter = findFriendNotBlocked(q, data, NULL, &status);

    Node toInsert = NodeCreate(data, NULL);
    if (!toInsert) {
        return ISRAELIQUEUE_ALLOC_FAILED;
    }

    NodeInsertAfter(insertAfter, &toInsert, status);
    return ISRAELIQUEUE_SUCCESS;
}

IsraeliQueueError IsraeliQueueAddFriendshipMeasure(IsraeliQueue q, FriendshipFunction function) {
    FriendshipFunction* friendships = (FriendshipFunction*)copyToMallocResize(
        q->m_friendships,
        (q->m_friendshipsLength + 1) * sizeof(FriendshipFunction),
        (q->m_friendshipsLength + 2) * sizeof(FriendshipFunction)
    );
    
    if (!friendships) {
        return ISRAELIQUEUE_ALLOC_FAILED;
    }

    friendships[q->m_friendshipsLength] = function;
    friendships[q->m_friendshipsLength + 1] = NULL;

    free(q->m_friendships);
    q->m_friendships = friendships;
    q->m_friendshipsLength++;

    return ISRAELIQUEUE_SUCCESS;
}

IsraeliQueueError IsraeliQueueUpdateFriendshipThreshold(IsraeliQueue q, int friendshipThreshold) {
    if (friendshipThreshold < 0) {
        return ISRAELIQUEUE_BAD_PARAM;
    }

    q->m_friendshipThreshold = friendshipThreshold;
    return ISRAELIQUEUE_SUCCESS;
}

IsraeliQueueError IsraeliQueueUpdateRivalryThreshold(IsraeliQueue q, int rivalryThreshold) {
    if (rivalryThreshold < 0) {
        return ISRAELIQUEUE_BAD_PARAM;
    }

    q->m_rivalryThreshold = rivalryThreshold;
    return ISRAELIQUEUE_SUCCESS;
}

/**Returns the number of elements of the given queue. If the parameter is NULL, 0
 * is returned.*/
int IsraeliQueueSize(IsraeliQueue q) {
    if (!q) {
        return 0;
    }

    int size = 0;
    for (Node curr = q->m_list; curr; curr = curr->m_next) {
        size++;
    }
    return size;
}

/**Removes and returns the foremost element of the provided queue. If the parameter
 * is NULL or a pointer to an empty queue, NULL is returned.*/
void* IsraeliQueueDequeue(IsraeliQueue q) {
    if (!q || !q->m_list) {
        return NULL;
    }

    Node first = q->m_list;
    q->m_list = q->m_list->m_next;
    void* data = first->m_data;
    free(first);
    return data;
}

bool IsraeliQueueContains(IsraeliQueue q, void* data) {
    if (!q || !data) {
        return false;
    }

    for (Node curr = q->m_list; curr; curr = curr->m_next) {
        if (q->m_compare(curr->m_data, data) == 0) {
            return true;
        }
    }
    return false;
}

IsraeliQueueError IsraeliQueueImprovePositionsRecursive(IsraeliQueue q, Node* nodePtr) {
    if (!nodePtr) {
        return ISRAELIQUEUE_SUCCESS;
    }

    // Recurse.
    IsraeliQueueError error = IsraeliQueueImprovePositionsRecursive(q, &(*nodePtr)->m_next);
    if (error != ISRAELIQUEUE_SUCCESS) {
        return error;
    }

    FriendStatus status = 0;
    Node* insertAfter = findFriendNotBlocked(q, &(*nodePtr)->m_data, *nodePtr, &status);

    NodeInsertAfter(insertAfter, nodePtr, status);

    return ISRAELIQUEUE_SUCCESS;
}

IsraeliQueueError IsraeliQueueImprovePositions(IsraeliQueue q) {
    return IsraeliQueueImprovePositionsRecursive(q, &q->m_list);
}

typedef struct MergeRet {
    FriendshipFunction* friendshipFunctions;
    int friendshipFunctionsSize;
    int friendshipThreshold;
    int rivalThreshold;
    bool error;
} MergeRet;

MergeRet MergeFriendshipsAndThresholds(IsraeliQueue* qarr) {
    MergeRet ret = {0};
    int i = 0;
    int j = 0;
    int k = 0;

    // Malloc friendship array.
    int friendshipsSize = 0;
    for (i = 0; qarr[i]; i++) {
        friendshipsSize += qarr[i]->m_friendshipsLength;
    }
    FriendshipFunction* friendships = malloc(sizeof(FriendshipFunction) * (friendshipsSize + 1));

    // Fail if malloc failed.
    if (!friendships) {
        ret.error = true;
        return ret;
    }
    
    // Initialize friendship array, and thresholds.
    int friendshipThreshold = 0;
    int rivalryThreshold = 0;
    for (i = 0; qarr[i]; i++) {
        friendshipThreshold += qarr[i]->m_friendshipThreshold;
        rivalryThreshold += qarr[i]->m_rivalryThreshold;
        for (j = 0; qarr[i]->m_friendships[j]; j++) {
            friendships[k++] = qarr[i]->m_friendships[j];
        }
    }
    friendships[k] = NULL;
    friendshipThreshold /= i;
    rivalryThreshold /= i;

    ret.friendshipFunctions = friendships;
    ret.friendshipFunctionsSize = friendshipsSize;
    ret.friendshipThreshold = friendshipThreshold;
    ret.rivalThreshold = rivalryThreshold;
    return ret;
}

IsraeliQueue IsraeliQueueMerge(IsraeliQueue* qarr, ComparisonFunction compare) {
    int i = 0;

    MergeRet results = MergeFriendshipsAndThresholds(qarr);

    IsraeliQueue mergedQueue = IsraeliQueueCreate(
            results.friendshipFunctions, compare,
            results.friendshipThreshold, results.rivalThreshold
    );

    // The constructor copies the friendships array, so we can free it.
    free(results.friendshipFunctions);

    // Fail if queue creation failed.
    if (!mergedQueue) {
        return NULL;
    }

    // Enqueue all items in the queues.
    for (i = 0; qarr[i]; i++) {
        void* item = IsraeliQueueDequeue(qarr[i]);
        if (item) {
            IsraeliQueueError error = IsraeliQueueEnqueue(mergedQueue, item);
            if (error) {
                IsraeliQueueDestroy(mergedQueue);
                return NULL;
            }
        }
    }

    return mergedQueue;
}
