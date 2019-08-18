#include "pdsi.h"

//-----------------------------------------------------------------------------
//**********   START OF FUNCTION DEFINITIONS FOR CLASS:  llist        *********
//-----------------------------------------------------------------------------
// The constructor for this function creates the sentinel for the
// double-linked list and points head to it.  The sentinel is a node that
// contains pointer to the beginning and to the end of the list.  Here both
// of these pointers point back to the sentinel itself.
//-----------------------------------------------------------------------------
llist::llist() {
    head = new node;       // Creates the sentinel with head pointing to it
    head->next = head;     // Sets the sentinels front pointer to itself
    head->previous = head; // Sets the sentinels rear pointer to itself
    size = 0;
}
//-----------------------------------------------------------------------------
// The destructor for this function deallocates all of the memory
// for the entire list.  In order to do this it must move through
// the list and delete each of these nodes.  Then it deletes the
// sentinel.
//-----------------------------------------------------------------------------
llist::~llist() {
    node *mover;                // The temporary pointer to perform the work

    mover = head->next;         // mover starts at the node after the sentinel
    while (mover != head) {     // This loop occurs until mover has come complete
        // circle and is pointing at the sentinel
        mover = mover->next;      // mover becomes the next node
        delete mover->previous;   // The previous node is then deleted
    }
    delete mover;               // Finally the sentinel is deleted
}
//-----------------------------------------------------------------------------
// The insert function places a new node with the integer value x
// between the sentinel and the first element in the list.  This
// effectively makes this new node the first element in the list.
//-----------------------------------------------------------------------------
void llist::insert(number x) {
    node *inserter;      // A new pointer to the node to be added
    inserter = new node; // This creates the node and points inserter to it
    inserter->key = x;   // The value in inserter is set to x

    inserter->next = head->next;        // The next field of the new node is
    // set to the node after the sentinel
    inserter->previous = head;          // The previous field is set to the
    // sentinel
    inserter->next->previous = inserter;// The previous field of the node after
    // inserter is set to inserter
    inserter->previous->next = inserter;// The sentinels next field is set to
    // inserter
    size++;                             // update size
}
int llist::get_size() {
    return size;
}

number* llist::returnArray() {

    node* cur;
    int i;
    number* A = new number[size];
    if (A != NULL) {
        cur = head->previous;
        i = 0;
        while (cur != head) {
            A[i] = cur->key;
            i++;
            cur = cur->previous;
        }
    }
    return A;

}
//-----------------------------------------------------------------------------
// The head_remove function removes the first node on the list and
// returns the value stored in that node
//-----------------------------------------------------------------------------
number llist::head_remove() {
    if (is_empty()) {
        return MISSING;
    }

    node *remover;
    number x;

    remover = head->next;
    x = remover->key;
    // First the previous field of the next node is set to head
    remover->next->previous = head;
    // Then the next field of head is set to the node after remover
    head->next = remover->next;
    size--;          //update size;
    delete remover;  // Finally the node can be deleted and function ends
    return x;  // The key is returned
}
//-----------------------------------------------------------------------------
// The tail_remove function removes the last nod on the list and
// returns the value stored in that node
//-----------------------------------------------------------------------------
number llist::tail_remove() {
    if (is_empty()) {
        return MISSING;
    }
    node *remover;
    number x;

    remover = head->previous;
    x = remover->key;
    //First the next field of the previous node is set to head
    remover->previous->next = head;
    //Then the previous field of head is set to the node before remover
    head->previous = remover->previous;
    size--;            //update size
    delete remover;    // Finally the node can be deleted
    return x; // The key is returned
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
node *llist::set_node(node *set, number x) {
    int error = 1;
    node *comparer;

    if (set == NULL)
        set = head->next;
    comparer = head->next;
    while (comparer != head) {
        if (comparer == set) {
            error = 0;
            break;
        }
        comparer = comparer->next;
    }

    if (error == 1) {
        return NULL;
    } else {
        if (set->key != MISSING) {
            set->key = x;
            return set->next;
        }
        else {
            //if the node is MISSING, then don't replace
            //that key.  instead, replace the first non-MISSING
            //node you come to.
            return set_node(set->next, x);
        }
    }
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int llist::is_empty() {
    if (head->next == head)
        return 1;
    else
        return 0;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void copy(llist &L1, const llist &L2) {
    while (!L1.is_empty())
        L1.tail_remove();

    node *comparer;
    comparer = L2.head->previous;
    while (comparer != L2.head) {
        L1.insert(comparer->key);
        comparer = comparer->previous;
    }
}
number llist::sumlist() {

    number sum = 0;

    node* cur;
    cur = head->previous;
    while (cur != head) {
        sum += cur->key;
        cur = cur->previous;
    }

    return sum;
}

void llist::sumlist(number &prev_sum, int sign) {

    //printf("in sumlist(number &prev_sum, int sign)\n");
    number sum = 0;

    node* cur;
    cur = head->previous;
    while (cur != head) {
        sum += cur->key;
        cur = cur->previous;
    }

    if (sign * sum > sign * prev_sum)
        prev_sum = sum;


    return;
}
number llist::maxlist() {
    number max = 0;

    node * cur;
    cur = head->previous;
    while (cur != head) {
        if (cur->key > max)
            max = cur->key;
        cur = cur->previous;
    }

    return max;
}
number llist::minlist() {
    number min = 0;

    node * cur;
    cur = head->previous;
    while (cur != head) {
        if (cur->key < min)
            min = cur->key;
        cur = cur->previous;
    }

    return min;
}
number llist::kthLargest(int k) {
    if (k < 1 || k > size)
        return MISSING;
    else if (k == 1)
        return minlist();
    else if (k == size)
        return maxlist();

    else {
        //place the list in an array for
        //easier selection
        number *A;
        int i;
        number return_value;
        node* cur = head->previous;
        A = new number[size];
        if (A != NULL) {
            for (i = 0; i < size; i++) {
                if (cur->key != MISSING)
                    A[i] = cur->key;
                cur = cur->previous;
            }
            select(A, 0, size - 1, k);

            return_value = A[k - 1];
            delete []A;
        }
        else {
            long_select(k);
        }
        return return_value;
    }
}
number llist::quartile(int q) {
    //q0 is the minimum
    if (q == 0)
        return minlist();
    //q4 is the maximum
    else if (q == 4)
        return maxlist();

    //q2 is the median
    else if (q == 2) {
        //if the size of the list is even, there is no exact median
        //so take the average of the two closest numbers
        if (size % 2 == 0) {
            double t1 = kthLargest(size / 2);
            double t2 = kthLargest(size / 2 + 1);
            return (t1 + t2) / 2;
        }
        else
            return kthLargest(1 + (size - 1) / 2);
    }

    //q1 is the first quartile, q3 is the third quartile
    else if (q == 1 || q == 3) {
        //if (size+1) is not divisble by 4, there is no exact quartiles
        //so take the weighted average of the two closest numbers
        int k;
        if ((k = ((size - 1) % 4)) != 0) {
            int bottom = (int)floor(q * (size - 1) / 4);
            double t1 = (4 - k) * kthLargest(bottom + 1);
            double t2 = (k) * kthLargest(bottom + 2);
            return (t1 + t2) / 4;
        }
        else
            return kthLargest(1 + q * (size - 1) / 4);
    }
    else
        return MISSING;
}
//safe percentile is a safer version of percentile that
//takes MISSING values into account
number llist::safe_percentile(double percentage) {
    llist temp;
    node* cur = head->next;
    while (cur != head) {
        if (cur->key != MISSING)
            temp.insert(cur->key);
        cur = cur->next;
    }
    return temp.percentile(percentage);
}
number llist::percentile(double percentage) {
    int k;
    //the argument may not be in correct demical
    //representation of a percentage, that is,
    //it may be a whole number like 25 instead of .25
    if (percentage > 1)
        percentage = percentage / 100;
    k = (int)(percentage * size);
    return kthLargest(k);
}
number llist::long_select(int k) {
    //haven't gotten around to doing this function yet.
    //it's pretty low priority for me.
    printf("Low Memory.\n");
    return MISSING;

}

void LeastSquares(int *x, number *y, int n, int sign, int verbose, number &slope, number &intercept)
{
    number sumX, sumX2, sumY, sumY2, sumXY;
    number SSX, SSY, SSXY;
    number xbar, ybar;

    number correlation = 0;
    number c_tol = 0.85;

    number max = 0;
    number max_diff = 0;
    int max_i = 0;

    number this_x, this_y;
    int i;

    sumX = 0;
    sumY = 0;
    sumX2 = 0;
    sumY2 = 0;
    sumXY = 0;
    for (i = 0; i < n; i++)
    {
        this_x = x[i];
        this_y = y[i];

        sumX += this_x;
        sumY += this_y;
        sumX2 += this_x * this_x;
        sumY2 += this_y * this_y;
        sumXY += this_x * this_y;
    }

    xbar = sumX / n;
    ybar = sumY / n;

    SSX = sumX2 - (sumX * sumX) / n;
    SSY = sumY2 - (sumY * sumY) / n;
    SSXY = sumXY - (sumX * sumY) / n;

    correlation = SSXY / (sqrt(SSX) * sqrt(SSY));

    if (verbose > 1 && (sign * correlation) < c_tol)
    {
        printf("original correlation = %.4f \n", correlation);
    }

    i = n - 1;
    while ((sign * correlation) < c_tol && i > 3)
    {
        //when the correlation is off, it appears better to
        //take the earlier sums rather than the later ones.
        this_x = x[i];
        this_y = y[i];

        sumX -= this_x;
        sumY -= this_y;
        sumX2 -= this_x * this_x;
        sumY2 -= this_y * this_y;
        sumXY -= this_x * this_y;

        SSX = sumX2 - (sumX * sumX) / i;
        SSY = sumY2 - (sumY * sumY) / i;
        SSXY = sumXY - (sumX * sumY) / i;

        xbar = sumX / i;
        ybar = sumY / i;

        correlation = SSXY / (sqrt(SSX) * sqrt(SSY));
        i--;
    }

    if (verbose > 1)
    {
        printf("final correlation =  %.4f\n\n", correlation);
    }
    slope = SSXY / SSX;

    n = i + 1;
    i = 0;
    for (i; i < n; i++)
    {
        if (sign * (y[i] - slope * x[i]) > sign * max_diff)
        {
            max_diff = y[i] - slope * x[i];
            max_i = i;
            max = y[i];
        }
    }
    intercept = max - slope * x[max_i];
} //end of LeastSquares()
