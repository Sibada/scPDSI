#include "pdsi.h"

#ifdef WIN32
#include <direct.h> // unccoment for windows machines.
#endif

//-----------------------------------------------------------------------------

int is_int(char *string, int length)
{
    int err = 1;
    for (int i = 0; i < length; i++)
        if (!isdigit(string[i]))
            err = 0;
    return err;
}
int is_flt(char *string, int length)
{
    int err = 1;
    for (int i = 0; i < length; i++)
        if (!isdigit(string[i]) && string[i] != '.')
            err = 0;
    return err;
}

//The numEntries() function returns the number of entries in the file
//It is used to check the T_normal file for the correct format because the
//program allows that filename to be used in place of either wk_T_normal
//or mon_T_normal.
int numEntries(FILE *in)
{
    int i = 0;
    float t;
    while (fscanf(in, "%f", &t) != EOF)
        i++;
    return i;
}

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
int partition(number a[], int left, int right)
{
    number val = a[right];
    int i = left - 1;
    for (int j = left; j < right; j++)
    {
        if (a[j] <= val)
        {
            i++;
            exch(a[i], a[j]);
        }
    }
    exch(a[i + 1], a[right]);
    return i + 1;
}

void select(number a[], int l, int r, int k)
{
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
void exch(number &x, number &y)
{
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
int dir_exists(char *dir)
{
    /*
    //---------------------------------------------------------------------------
    //unix version
    char command[270];
    int result = 9;

    //test to see if the directory exists.
    sprintf(command, "test -d %s",dir);
    result = system(command);
    int status = result == 0 ? 1 : -1;
    return status;
    */
    //---------------------------------------------------------------------------
    //Windows version
    FILE *test;
    char test_file[128];

    //make sure the last letter of the directory is a slash (/).
    if (dir[strlen(dir) - 1] != '/')
        strcat(dir, "/");

    //test to see if the directory exists.
    //there is probably a better way to do this,
    //but I don't know what it is and this works fine.
    strcpy(test_file, dir);
    strcat(test_file, "test.file");
    test = fopen(test_file, "w");

    if (test == NULL)
    {
        //the file could not be opened,
        //so the directory does not exist
        return -1;
    }
    fclose(test);
    remove(test_file);
    return 1;
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
int create_dir(char *path)
{
    unsigned int i = 0;
    char my_path[128];
    char dir[128];
    int return_value = 1;

    //make sure the last letter of the directory is a slash (/).
    strcpy(my_path, path);
    if (path[strlen(path) - 1] != '/')
        strcat(my_path, "/");

    while (i < strlen(my_path))
    {
        dir[i] = my_path[i];
        if (my_path[i] == '/')
        {
            dir[i + 1] = '\0';
            //check to see if this directory exists
            if (dir_exists(dir) == -1)
            {
                return_value = 0;

#ifdef WIN32
                _mkdir(dir); // if(_mkdir(dir) != 0) return -1;
#else
                // linux or mac
                char command[270];
                sprintf(command, "mkdir %s -p", dir);
                system(command);
#endif

                if (dir_exists(dir) == -1)
                    return -1;
            }
        }
        i++;
    }
    return return_value;
}
