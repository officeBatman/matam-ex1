#include "IsraeliQueue.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct Node_t* Node;

struct Node_t {
    void* data;
    int friendsCalledOver;
    int rivalsBlocked;
    Node next;
};

struct IsraeliQueue_t {
    Node list;
    FriendshipFunction* friendships;
    int friendshipsSize;
    ComparisonFunction compare;
    int friendshipThreshold;
    int rivalryThreshold;
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

    memcpy(ret, data, min(dataSize, outSize));

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
    for (int i = 0; i < q->friendshipsSize; i++) {
        int friendshipNumber = q->friendships[i](data1, data2);
        friendshipSum += friendshipNumber;

        // Exit early if the friendship is friendly enough.
        if (friendshipSum > q->friendshipThreshold) {
            return FRIEND;
        }
    }

    // Check if the objects are enemies.
    float friendshipAverage = (float)friendshipSum / q->friendshipsSize;
    if (friendshipAverage < q->rivalryThreshold) {
        return RIVAL;
    }

    return NEUTRAL;
}


// === Node Functions ===

// Initialize a new node in place.
void NodeInit(Node node, void* data, Node next) {
    node->data = data;
    node->friendsCalledOver = 0;
    node->rivalsBlocked = 0;
    node->next = next;
}

// Create a new node and malloc it.
Node NodeCreate(void* data, Node next) {
    Node ret = (Node)malloc(sizeof(struct Node_t));
    NodeInit(ret, data, next);
    return ret;
}

Node FindFriendNotBlocked(IsraeliQueue q, void* data, Node stop, Node* outBlockingRival, Node* outLast) {
    // Iterativly, find the first friend, and if it is blocked, start over.
    Node friend = NULL;
    Node curr = NULL;
    for (curr = q->list; curr != NULL && curr != stop; curr = curr->next) {
        FriendStatus status = getFriendshipStatus(q, data, curr->data);
        if (status == FRIEND && curr->friendsCalledOver < FRIEND_QUOTA && !friend) {
            friend = curr;
        } else if (status == RIVAL && curr->rivalsBlocked < RIVAL_QUOTA && friend) {
            friend = NULL;
            if (outBlockingRival) {
                *outBlockingRival = curr;
            }
        }
    }

    if (outLast) {
        *outLast = curr;
    }

    return friend;
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

    ret->list = NULL;
    ret->friendships = friendshipsCopied;
    ret->friendshipsSize = functions;
    ret->compare = compare;
    ret->friendshipThreshold = friendshipThreshold;
    ret->rivalryThreshold = rivalryThreshold;
    return ret;
}

IsraeliQueue IsraeliQueueClone(IsraeliQueue q) {
    return IsraeliQueueCreate(q->friendships, q->compare, q->friendshipThreshold, q->rivalryThreshold);
}

void IsraeliQueueDestroy(IsraeliQueue q) {
    // Exit early if the queue is already NULL.
    if (!q) {
        return;
    }

    free(q->friendships);
    free(q);
}

IsraeliQueueError IsraeliQueueEnqueue(IsraeliQueue q, void* data) {
    Node end = NULL;
    Node blockingRival = NULL;
    Node friend = FindFriendNotBlocked(q, data, NULL, &blockingRival, &end);

    Node insertAfter = end;
    Node next = NULL;

    if (friend) {
        insertAfter = friend;
        friend->friendsCalledOver++;
    } 

    if (blockingRival) {
        blockingRival->rivalsBlocked++;
    }

    Node toInsert = NodeCreate(data, next);
    if (!toInsert) {
        return ISRAELIQUEUE_ALLOC_FAILED;
    }

    insertAfter->next = NodeCreate(data, next);

    return ISRAELIQUEUE_SUCCESS;
}

IsraeliQueueError IsraeliQueueAddFriendshipMeasure(IsraeliQueue q, FriendshipFunction function) {
    FriendshipFunction* friendships = (FriendshipFunction*)copyToMallocResize(
        q->friendships,
        (q->friendshipsSize + 1) * sizeof(FriendshipFunction),
        (q->friendshipsSize + 2) * sizeof(FriendshipFunction)
    );
    
    if (!friendships) {
        return ISRAELIQUEUE_ALLOC_FAILED;
    }

    friendships[q->friendshipsSize] = function;
    friendships[q->friendshipsSize + 1] = NULL;

    free(q->friendships);
    q->friendships = friendships;
    q->friendshipsSize++;

    return ISRAELIQUEUE_SUCCESS;
}

IsraeliQueueError IsraeliQueueUpdateFriendshipThreshold(IsraeliQueue q, int friendshipThreshold) {
    if (friendshipThreshold < 0) {
        return ISRAELIQUEUE_BAD_PARAM;
    }

    q->friendshipThreshold = friendshipThreshold;
    return ISRAELIQUEUE_SUCCESS;
}

IsraeliQueueError IsraeliQueueUpdateRivalryThreshold(IsraeliQueue q, int rivalryThreshold) {
    if (rivalryThreshold < 0) {
        return ISRAELIQUEUE_BAD_PARAM;
    }

    q->rivalryThreshold = rivalryThreshold;
    return ISRAELIQUEUE_SUCCESS;
}

/**Returns the number of elements of the given queue. If the parameter is NULL, 0
 * is returned.*/
int IsraeliQueueSize(IsraeliQueue q) {
    if (!q) {
        return 0;
    }

    int size = 0;
    for (Node curr = q->list; curr; curr = curr->next) {
        size++;
    }
    return size;
}

/**Removes and returns the foremost element of the provided queue. If the parameter
 * is NULL or a pointer to an empty queue, NULL is returned.*/
void* IsraeliQueueDequeue(IsraeliQueue q) {
    if (!q || !q->list) {
        return NULL;
    }

    Node first = q->list;
    q->list = q->list->next;
    void* data = first->data;
    free(first);
    return data;
}

bool IsraeliQueueContains(IsraeliQueue q, void* data) {
    if (!q || !data) {
        return false;
    }

    for (Node curr = q->list; curr; curr = curr->next) {
        if (q->compare(curr->data, data) == 0) {
            return true;
        }
    }
    return false;
}

IsraeliQueueError _IsraeliQueueImprovePositions(IsraeliQueue q, Node node) {
    if (!node) {
        return ISRAELIQUEUE_SUCCESS;
    }

    // Recurse.
    IsraeliQueueError error = _IsraeliQueueImprovePositions(q, node->next);
    if (error != ISRAELIQUEUE_SUCCESS) {
        return error;
    }

    Node blockingRival = q->findRival(q, node->data, node);
    FindFriendNotBlocked(q, node->data, node, );
}

/**Advances each item in the queue to the foremost position accessible to it,
 * from the back of the queue frontwards.*/
IsraeliQueueError IsraeliQueueImprovePositions(IsraeliQueue q) {
    _IsraeliQueueImprovePositions(q, q->list);
}

/**@param q_arr: a NULL-terminated array of IsraeliQueues
 * @param ComparisonFunction: a comparison function for the merged queue
 *
 * Merges all queues in q_arr into a single new queue, with parameters the parameters described
 * in the exercise. Each queue in q_arr enqueues its head in the merged queue, then lets the next
 * one enqueue an item, in the order defined by q_arr. In the event of any error during execution, return NULL.*/
IsraeliQueue IsraeliQueueMerge(IsraeliQueue*,ComparisonFunction);
