#include "pdsi.h"

inline int pdsi::is_int(char *string, int length) {
    int err = 1;
    for (int i = 0; i < length; i++)
        if (!isdigit(string[i])) err = 0;
    return err;
}
inline int pdsi::is_flt(char *string, int length) {
    int err = 1;
    for (int i = 0; i < length; i++)
        if (!isdigit(string[i]) && string[i] != '.') err = 0;
    return err;
}

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
    }
    else {
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
            int bottom = (int)std::floor(q * (size - 1) / 4);
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
    //printf("Low Memory.\n");
    return MISSING;

}
//-----------------------------------------------------------------------------
//**********   CLOSE OF FUNCTION DEFINITIONS FOR CLASS:  llist        *********
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//The numEntries() function returns the number of entries in the file
//It is used to check the T_normal file for the correct format because the
//program allows that filename to be used in place of either wk_T_normal
//or mon_T_normal.
int numEntries(FILE *in) {
    int i = 0;
    float t;
    while (fscanf(in, "%f", &t) != EOF)
        i++;
    return i;
}
//-----------------------------------------------------------------------------
//These three functions, partition(), select(), and exch() are used to select
//the kth largest number in an array.
//Partition partitions a subarray around the a key such that all entries above
//  the key are greater than the key, and all entries below are less than it.
//  The rightmost element in the subarray is used as the key.
//Select arranges the array in such a way that the kth largest item in the
//  array is in the kth spot, that is at index # (k-1).
//Exch simply switches the values of the two arguments.
//To possibly speed up the process, these three functions could be combined,
//  meaning there would be far fewer function calls.
//-----------------------------------------------------------------------------
int partition(number a[], int left, int right) {
    number val = a[right];
    int i = left - 1;
    for (int j = left; j < right; j++) {
        if (a[j] <= val) {
            i++;
            exch(a[i], a[j]);
        }
    }
    exch(a[i + 1], a[right]);
    return i + 1;
}

void select(number a[], int l, int r, int k) {
    int i;
    if (r <= l)
        return;

    i = partition(a, l, r);
    if (i > k - 1)
        select(a, l, i - 1, k);
    else if (i < k - 1)
        select(a, i + 1, r, k);
    else
        return;
}
void exch(number &x, number &y) {
    number temp;
    temp = x;
    x = y;
    y = temp;
}
//-----------------------------------------------------------------------------
//The dir_exists function is a function used to test to see if a directory
//exists.
//This function is platform-specific, that is, it must be changed to be
//compatible with the platform (Windows PC, Unix based machine, ect).
//Returns -1 if the directory does not exist
//Returns 1 if the directory exists.
//-----------------------------------------------------------------------------
int dir_exists(char *dir) {
    /*
    //---------------------------------------------------------------------------
    //unix version
    //---------------------------------------------------------------------------
    char command[270];
    int result = 9;

    //test to see if the directory exists.
    sprintf(command, "test -d %s",dir);
    result = system(command);
    if(result == 0)
      return 1;
    else
      return -1;
    */
    //---------------------------------------------------------------------------
    //Windows version
    //---------------------------------------------------------------------------
    /*
    FILE* test;
    char test_file[128];

    //make sure the last letter of the directory
    //is a slash (/).
    if(dir[strlen(dir)-1] != '/')
      strcat(dir,"/");

    //test to see if the directory exists.
    //there is probably a better way to do this,
    //but I don't know what it is and this works fine.
    strcpy(test_file,dir);
    strcat(test_file, "test.file");
    test = fopen(test_file, "w");

    if(test == NULL){
      //the file could not be opened,
      //so the directory does not exist
      return -1;
    }
    fclose(test);
    remove(test_file);
    return 1;
    //---------------------------------------------------------------------------
    */
    return 0;
}
//-----------------------------------------------------------------------------
//The create_dir function will create all directories given in the argument
//"path".
//This function is platform-specific, that is, it must be changed to be
//compatible with the platform (Windows PC, Unix based machine, ect).
//The windows verion of this function requires the additional include file
// <direct.h>
//Returns -1 upon failure
//Returns 0 if the directory was successfully created
//Returns 1 if the directory already exists.
//-----------------------------------------------------------------------------
int create_dir(char *path) {
    return 0;
}
