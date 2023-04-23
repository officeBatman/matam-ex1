#include "IsraeliQueue.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

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
    int m_friendshipsSize;
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
    for (int i = 0; i < q->m_friendshipsSize; i++) {
        int friendshipNumber = q->m_friendships[i](data1, data2);
        friendshipSum += friendshipNumber;

        // Exit early if the friendship is friendly enough.
        if (friendshipSum > q->m_friendshipThreshold) {
            return FRIEND;
        }
    }

    // Check if the objects are enemies.
    float friendshipAverage = (float)friendshipSum / q->m_friendshipsSize;
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

Node FindFriendNotBlocked(IsraeliQueue q, void* data, Node stop, Node* outBlockingRival, Node* outLast) {
    // Iterativly, find the first friend, and if it is blocked, start over.
    Node friend = NULL;
    Node curr = NULL;
    for (curr = q->m_list; curr != NULL && curr != stop; curr = curr->m_next) {
        FriendStatus status = getFriendshipStatus(q, data, curr->m_data);
        if (status == FRIEND && curr->m_friendsCalledOver < FRIEND_QUOTA && !friend) {
            friend = curr;
        } else if (status == RIVAL && curr->m_rivalsBlocked < RIVAL_QUOTA && friend) {
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

    ret->m_list = NULL;
    ret->m_friendships = friendshipsCopied;
    ret->m_friendshipsSize = functions;
    ret->m_compare = compare;
    ret->m_friendshipThreshold = friendshipThreshold;
    ret->m_rivalryThreshold = rivalryThreshold;
    return ret;
}

IsraeliQueue IsraeliQueueClone(IsraeliQueue q) {
    return IsraeliQueueCreate(q->m_friendships, q->m_compare, q->m_friendshipThreshold, q->m_rivalryThreshold);
}

void IsraeliQueueDestroy(IsraeliQueue q) {
    // Exit early if the queue is already NULL.
    if (!q) {
        return;
    }

    free(q->m_friendships);
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
        friend->m_friendsCalledOver++;
    } else if (blockingRival) {
        insertAfter = blockingRival;
        blockingRival->m_rivalsBlocked++;
    }

    Node toInsert = NodeCreate(data, next);
    if (!toInsert) {
        return ISRAELIQUEUE_ALLOC_FAILED;
    }

    insertAfter->m_next = NodeCreate(data, next);

    return ISRAELIQUEUE_SUCCESS;
}

IsraeliQueueError IsraeliQueueAddFriendshipMeasure(IsraeliQueue q, FriendshipFunction function) {
    FriendshipFunction* friendships = (FriendshipFunction*)copyToMallocResize(
        q->m_friendships,
        (q->m_friendshipsSize + 1) * sizeof(FriendshipFunction),
        (q->m_friendshipsSize + 2) * sizeof(FriendshipFunction)
    );
    
    if (!friendships) {
        return ISRAELIQUEUE_ALLOC_FAILED;
    }

    friendships[q->m_friendshipsSize] = function;
    friendships[q->m_friendshipsSize + 1] = NULL;

    free(q->m_friendships);
    q->m_friendships = friendships;
    q->m_friendshipsSize++;

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

IsraeliQueueError _IsraeliQueueImprovePositions(IsraeliQueue q, Node* nodePtr) {
    if (!nodePtr) {
        return ISRAELIQUEUE_SUCCESS;
    }

    // Recurse.
    IsraeliQueueError error = _IsraeliQueueImprovePositions(q, &(*nodePtr)->m_next);
    if (error != ISRAELIQUEUE_SUCCESS) {
        return error;
    }

    Node blockingRival = NULL;
    Node last = NULL;
    Node friend = FindFriendNotBlocked(q, &(*nodePtr)->m_data, *nodePtr, &blockingRival, &last);

    // Insert. Not specified: assume counters should increase.
    Node insertAfter = NULL;
    if (friend) {
        insertAfter = friend;
        friend->m_friendsCalledOver++;
    } else if (blockingRival) {
        insertAfter = blockingRival;
        blockingRival->m_rivalsBlocked++;
    }
    
    if (insertAfter) {
        Node temp = (*nodePtr)->m_next;
        (*nodePtr)->m_next = insertAfter->m_next;
        insertAfter->m_next = *nodePtr;
        *nodePtr = temp;
    }

    return ISRAELIQUEUE_SUCCESS;
}

IsraeliQueueError IsraeliQueueImprovePositions(IsraeliQueue q) {
    return _IsraeliQueueImprovePositions(q, &q->m_list);
}

/**@param q_arr: a NULL-terminated array of IsraeliQueues
 * @param ComparisonFunction: a comparison function for the merged queue
 *
 * Merges all queues in q_arr into a single new queue, with parameters the parameters described
 * in the exercise. Each queue in q_arr enqueues its head in the merged queue, then lets the next
 * one enqueue an item, in the order defined by q_arr. In the event of any error during execution, return NULL.*/
IsraeliQueue IsraeliQueueMerge(IsraeliQueue* qarr, ComparisonFunction compare) {
    int i = 0;
    int j = 0;
    int k = 0;

    // Malloc friendship array.
    int friendshipsSize = 0;
    for (i = 0; qarr[i]; i++) {
        friendshipsSize += qarr[i]->m_friendshipsSize;
    }
    FriendshipFunction* friendships = malloc(sizeof(FriendshipFunction) * (friendshipsSize + 1));

    // Fail if malloc failed.
    if (!friendships) {
        return NULL;
    }
    
    // Initialize friendship array, and thresholds.
    float friendshipThreshold = 0;
    float rivalryThreshold = 0;
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

    IsraeliQueue mergedQueue = IsraeliQueueCreate(friendships, compare, friendshipThreshold, rivalryThreshold);

    // The constructor copies the friendships array, so we can free it.
    free(friendships);

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
