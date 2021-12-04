#include <stdio.h>
#include <stdlib.h>

char store_w[] = "store";
char load_w[] = "load";

char instruction[10];

int isStore(char *input)
{
    int i = 0;
    while (1)
    {
        if (input[i] != store_w[i])
        {
            return 0;
        }
        else
        {
            if (store_w[i] == 0)
            {
                return 1;
            }
        }
        i++;
    }
    return 1;
}

struct dataInCache
{
    int tag;
    int whichWay;
    char *data;
};

int cache_size;
int associativity;
int block_size;
int frame_num;
int set_num;

const int address_bit = 24;

const int mem_size = 16 * 1024 * 1024;

char main_mem[mem_size];

void init_mem(char *mem, int mem_size)
{
    for (int i = 0; i < mem_size; i++)
    {
        main_mem[i] = 0;
    }
}

int get_log_2(int num)
{
    int r = 0;
    while (num >>= 1)
    {
        r++;
    }
    return r;
}

void copy_mem(char *a, char *b, int size)
// Copy b into a
{
    for (int i = 0; i < size; i++)
    {
        a[i] = b[i];
    }
}

int main(int argc, char *argv[])
{
    FILE *input_file = fopen(argv[1], "r");

    // Read the configuration
    cache_size = atoi(argv[2]) * 1024;
    associativity = atoi(argv[3]);
    block_size = atoi(argv[4]);

    frame_num = cache_size / block_size;

    init_mem(main_mem, mem_size);

    // Compute how many sets
    set_num = frame_num / associativity;

    // Initiate the cache
    struct dataInCache cache[set_num][associativity];
    for (int i = 0; i < set_num; i++)
    {
        for (int j = 0; j < associativity; j++)
        {
            cache[i][j].tag = 0;
            cache[i][j].whichWay = -1;
            cache[i][j].data = (char *)malloc(block_size);
        }
    }

    int address;
    int bytes_needed;

    int isHit;
    int wayIsHit;

    while (fscanf(input_file, "%s", instruction) != EOF)
    {
        int offset_bit = get_log_2(block_size);
        int index_bit = get_log_2(set_num);

        int tag_bit = address_bit - offset_bit - index_bit;

        fscanf(input_file, "%x", &address);
        fscanf(input_file, "%d", &bytes_needed);

        // Determine whether it is a hit
        int offset = address & (1 << offset_bit);
        int index = (address >> offset_bit) & (1 << index_bit);
        int tag = address >> (offset_bit + index_bit);

        isHit = 0;
        for (int i = 0; i < associativity; i++)
        {
            if (cache[index][i].tag == tag)
            {
                isHit = 1;
                wayIsHit = i;
                break;
            }
        }

        if (isStore(instruction))
        {
            // write into main memory
            for (int i = 0; i < bytes_needed; i++)
            {
                char input;
                fscanf(input_file, "%2hhx", &input);
                main_mem[address + i] = input;
            }

            // Deal with LRU
            if (isHit)
            {
                for (int i = 0; i < associativity; i++)
                {
                    if (cache[index][i].whichWay < wayIsHit && cache[index][i].whichWay >= 0)
                    {
                        cache[index][i].whichWay++;
                    }
                    else if (cache[index][i].tag == tag)
                    {
                        cache[index][i].whichWay = 0;
                    }
                }
                for (int i = 0; i < associativity; i++)
                {
                    if (cache[index][i].whichWay == 0)
                    {
                        copy_mem(cache[index][i].data, &main_mem[address], bytes_needed);
                        break;
                    }
                }
                printf("%s 0x%x hit\n", instruction, address);
            }
            else
            {
                printf("%s 0x%x miss\n", instruction, address);
            }
        }
        else
        {
            if (isHit)
            {
                for (int i = 0; i < associativity; i++)
                {
                    if (cache[index][i].whichWay < wayIsHit && cache[index][i].whichWay >= 0)
                    {
                        cache[index][i].whichWay++;
                    }
                    else if (cache[index][i].tag == tag)
                    {
                        cache[index][i].whichWay = 0;
                    }
                }

                printf("%s 0x%x hit ", instruction, address);

                for (int ass = 0; ass < associativity; ass++)
                {
                    if (cache[index][ass].whichWay == 0)
                    {

                        for (int i = 0; i < bytes_needed; i++)
                        {
                            printf("%02hhx", cache[index][ass].data[i]);
                        }
                        printf("\n");
                    }
                    break;
                }
            }
            else
            {
                for (int i = 0; i < associativity; i++)
                {
                    if (cache[index][i].whichWay > -1 && cache[index][i].whichWay < associativity - 1)
                    {
                        cache[index][i].whichWay++;
                    }
                    else
                    {
                        cache[index][i].whichWay = 0;
                        cache[index][i].tag = tag;
                        copy_mem(cache[index][i].data, &main_mem[address], bytes_needed);
                    }
                }

                for (int i = 0; i < associativity; i++)
                {
                    if (cache[index][i].whichWay == 0)
                    {
                        printf("%s 0x%x miss", instruction, address);
                        for (int s = 0; s < bytes_needed; s++)
                        {
                            printf("%02hhx", cache[index][i].data[s]);
                        }

                        break;
                    }
                }
            }
        }
    }

    fclose(input_file);

    for (int i = 0; i < set_num; i++)
    {
        for (int j = 0; j < associativity; j++)
        {
            free(cache[i][j].data);
        }
    }

    return 0;
}
