#include <stdio.h>
#include <stdlib.h>

int number;
int address_bits;
int page_size;
int vp_num;
int virt_add;

int get_log_2(int num)
{
    int r = 0;
    while (num >>= 1)
    {
        r++;
    }
    return r;
}

int main(int argc, char *argv[])
{
    virt_add = strtol(argv[2], NULL, 16);

    FILE *input_file = fopen(argv[1], "r");
    fscanf(input_file, "%d", &address_bits);
    fscanf(input_file, "%d", &page_size);

    // Compute number of virtual pages
    vp_num = (1 << address_bits) / page_size;

    int pagetable[vp_num];

    // Read page table
    for (int i = 0; i < vp_num; i++)
    {
        fscanf(input_file, "%d", &number);
        pagetable[i] = number;
    }

    int vp_bit = get_log_2(vp_num);
    int offset_bit = address_bits - vp_bit;

    int vpn_idx = virt_add >> offset_bit;
    int offset = virt_add - (vpn_idx << offset_bit);

    if (pagetable[vpn_idx] == -1)
    {
        printf("PAGEFAULT\n");
        return 0;
    }

    int vpn = pagetable[vpn_idx];
    int phy_add = (vpn << offset_bit) | offset;
    printf("%x\n", phy_add);

    return 0;
}
