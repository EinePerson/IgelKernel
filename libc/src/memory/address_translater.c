//
// Created by igel on 26.03.25.
//

#include <stdio.h>
#include <stdlib.h>
#include <kernel/memory.h>

#include "memory.h"


/**
 * these 4 structs create a search tree to map a virtual to a physical address
 * the first (uint16_t) is the index,the next two are the child elements
 * the 4th entry is the pointer to the next lower structer if the index matches
 * the optional 5th entry is the page_size bit,if this is true the value in the 4th element is to be interpreted as base which is the physical address
 */
struct pt_tree {
    uint16_t pt;
    struct pt_tree *left;
    struct pt_tree *right;
    void* base;
};

/**
 * these 4 structs create a search tree to map a virtual to a physical address
 * the first (uint16_t) is the index,the next two are the child elements
 * the 4th entry is the pointer to the next lower structer if the index matches
 * the optional 5th entry is the page_size bit,if this is true the value in the 4th element is to be interpreted as base which is the physical address
 */
struct pd_tree {
    uint16_t pd;
    struct pd_tree* left;
    struct pd_tree* right;
    struct pt_tree *next;
    bool page_size;
};

/**
 * these 4 structs create a search tree to map a virtual to a physical address
 * the first (uint16_t) is the index,the next two are the child elements
 * the 4th entry is the pointer to the next lower structer if the index matches
 * the optional 5th entry is the page_size bit,if this is true the value in the 4th element is to be interpreted as base which is the physical address
 */
struct pdpt_tree {
    uint16_t pdpt;
    struct pdpt_tree *left;
    struct pdpt_tree *right;
    struct pd_tree* next;
    bool page_size;
};

/**
 * these 4 structs create a search tree to map a virtual to a physical address
 * the first (uint16_t) is the index,the next two are the child elements
 * the 4th entry is the pointer to the next lower structer if the index matches
 * the optional 5th entry is the page_size bit,if this is true the value in the 4th element is to be interpreted as base which is the physical address
 */
struct pml4_tree {
    uint16_t pml4;
    struct pml4_tree *left;
    struct pml4_tree *right;
    struct pdpt_tree* next;
};

/**
 * This 5th entry is to distinguish the virtual address space,this contains the value stored in cr3 for each process
 */
struct cr3_array {
    uint64_t cr3;
    struct cr3_array* left;
    struct cr3_array* right;
    struct pml4_tree* next;
};

/**
 *
 * @param cr3 the value that cr3 holds
 * @param current the current itteration point(cr3_root at start)
 * @return the pml4 entry that is pointed to by cr3
 */
struct cr3_array* getRoot(uint64_t cr3,struct cr3_array* current) {
    if (cr3 == current->cr3)return current;
    if (cr3 < current->cr3)return getRoot(cr3, current->right);
    if (cr3 > current->cr3)return getRoot(cr3, current->left);
}

/**
 *
 * @param pml4_index the index in the pml4 table
 * @param current the current iteration pointer
 * @return the pdpt pointed to
 */
struct pml4_tree* getPML4(uint16_t pml4_index,struct pml4_tree* current) {
    if (pml4_index == current->pml4)return current;
    if (pml4_index < current->pml4)return getPML4(pml4_index, current->right);
    if (pml4_index > current->pml4)return getPML4(pml4_index, current->left);
}

/**
 *
 * @param pdpt_index the index in the pdpt table
 * @param current the current iteration index
 * @return the pointer to the entry,as the entry can also be a physical address one has to check first if page_size is set
 */
struct pdpt_tree* getPDPT(uint16_t pdpt_index, struct pdpt_tree* current) {
    if (pdpt_index == current->pdpt) return current;
    if (pdpt_index < current->pdpt) return getPDPT(pdpt_index, current->right);
    if (pdpt_index > current->pdpt) return getPDPT(pdpt_index, current->left);
}

/**
 *
 * @param pd_index the index in the pd table
 * @param current the current iteration index
 * @return the pointer to the entry,as the entry can also be a physical address one has to check first if page_size is set
 */
struct pd_tree* getPD(uint16_t pd_index, struct pd_tree* current) {
    if (pd_index == current->pd) return current;
    if (pd_index < current->pd) return getPD(pd_index, current->right);
    if (pd_index > current->pd) return getPD(pd_index, current->left);
}

/**
 *
 * @param pt_index the index in the pt table
 * @param current the current itteration index
 * @return the physical address
 */
struct pt_tree* getBase(uint16_t pt_index,struct pt_tree* current) {
    if (pt_index == current->pt)return current;
    if (pt_index < current->pt)return getBase(pt_index, current->right);
    if (pt_index > current->pt)return getBase(pt_index, current->left);
}

///this is the root of the full tree
struct cr3_array* cr3_root;



void* translate_to_physicall(void* virt) {
    return get_physical_address(virt,(uint64_t) get_page_pointer());
}

/**
 *
 * @param virt the virtual address to convert
 * @param cr3 the current value of cr3
 * @return the corresponding physical address
 */
void* get_physical_address(void* virt,uint64_t cr3){
    if (cr3_root == 0) {
        printf("Usage of uninitialized address translater");
        abort();
    }

    uint64_t virutal_address = (uint64_t)virt;
    uint64_t pml4_index = (virutal_address >> 39) & 0x1FF;
    uint64_t pdpt_index = (virutal_address >> 30) & 0x1FF;
    uint64_t pd_index   = (virutal_address >> 21) & 0x1FF;
    uint64_t pt_index   = (virutal_address >> 12) & 0x1FF;

    struct cr3_array* root = getRoot(cr3, cr3_root);
    if(root == 0){
        printf("Invalid virtual address");
        abort();
    }

    struct pml4_tree* pdpt = getPML4(pml4_index, root->next);
    if (pdpt == 0) {
        printf("Invalid virtual address");
        abort();
    }

    struct pdpt_tree* pd_ptr = getPDPT(pdpt_index,pdpt->next);
    if (pd_ptr == 0) {
        printf("Invalid virtual address");
        abort();
    }
    if (pd_ptr->page_size)return (void*) pd_ptr->next;
    struct pd_tree *pd = pd_ptr->next;

    struct pd_tree *pt_ptr = getPD(pd_index, pd);
    if (pt_ptr == 0) {
        printf("Invalid virtual address");
        abort();
    }
    if (pt_ptr->page_size)return (void*) pt_ptr->next;
    struct pt_tree *pt = pt_ptr->next;

    struct pt_tree* base = getBase(pt_index, pt);
    return base->base;
}

void addCR3(uint64_t cr3) {
    struct cr3_array* arr = malloc(sizeof(struct cr3_array));;
    arr->next = 0;
    arr->cr3 = cr3;
    if (cr3_root == 0) {
        cr3_root = arr;
    }else {
        struct cr3_array* prev = 0;
        struct cr3_array* curr = cr3_root;
        while (curr != 0) {
            if (curr->cr3 == cr3) {
                printf("Address space (CR3) already present");
                abort();
            }
            prev = curr;
            if (cr3 < curr->cr3)curr = curr->right;
            else if (cr3 > curr->cr3)curr = curr->left;
        }

        if (cr3 < prev->cr3)prev->right = arr;
        else if (cr3 > prev->cr3)prev->left = arr;
    }
}

void addPML4(uint64_t cr3,uint16_t pml4_index) {
    if (cr3_root == 0) {
        printf("Usage of uninitialized address translater");
        abort();
    }
    struct pml4_tree* pml4 = malloc(sizeof(struct pml4_tree));
    pml4->pml4 = pml4_index;
    pml4->next = 0;

    struct cr3_array* cr3_arr = getRoot(cr3, cr3_root);
    if (cr3_arr->next == 0) {
        cr3_arr->next = pml4;
        return;
    }
    struct pml4_tree* prev = 0;
    struct pml4_tree* curr = cr3_arr->next;
    while (curr != 0) {
        prev = curr;
        if (pml4_index == curr->pml4) {
            printf("PML4 present");
            abort();
        }
        if (pml4_index < curr->pml4)curr = curr->right;
        else if (pml4_index > curr->pml4)curr = curr->left;
    }

    if (pml4_index < prev->pml4)prev->right = pml4;
    else if (pml4_index > prev->pml4)prev->left = pml4;
}

void addPdpt(uint64_t cr3,uint16_t pml4_index,uint16_t pdpt_index) {
    if (cr3_root == 0) {
        printf("Usage of uninitialized address translater");
        abort();
    }
    struct pdpt_tree* pdpt = malloc(sizeof(struct pdpt_tree));
    pdpt->pdpt = pdpt_index;
    pdpt->page_size = false;
    pdpt->next = 0;

    struct pml4_tree* pml4 = getRoot(cr3,cr3_root)->next;
    struct pml4_tree* pml4_root = getPML4(pml4_index,pml4);
    if (pml4_root->next == 0) {
        pml4_root->next = pdpt;
        return;
    }
    struct pdpt_tree* prev = 0;
    struct pdpt_tree* curr = pml4_root->next;
    while (curr != 0) {
        prev = curr;
        if (curr->pdpt == pdpt_index) {
            printf("PDPT present");
            abort();
        }
        if (pdpt_index < curr->pdpt)curr = curr->right;
        else if (pdpt_index > curr->pdpt)curr = curr->left;
    }

    if (pdpt_index < prev->pdpt)prev->right = pdpt;
    else if (pdpt_index > prev->pdpt)prev->left = pdpt;
}

void addPdptPage(uint64_t cr3,uint16_t pml4_index,uint16_t pdpt_index,void* base) {
    if (cr3_root == 0) {
        printf("Usage of uninitialized address translater");
        abort();
    }
    struct pdpt_tree* pdpt = malloc(sizeof(struct pdpt_tree));
    pdpt->pdpt = pdpt_index;
    pdpt->page_size = true;
    pdpt->next = (struct pd_tree*) base;

    struct pml4_tree* pml4 = getRoot(cr3,cr3_root)->next;
    struct pml4_tree* pml4_root = getPML4(pml4_index,pml4);
    if (pml4_root->next == 0) {
        pml4_root->next = pdpt;
        return;
    }
    struct pdpt_tree* prev = 0;
    struct pdpt_tree* curr = pml4_root->next;
    while (curr != 0) {
        prev = curr;
        if (curr->pdpt == pdpt_index) {
            printf("PDPT present");
            abort();
        }
        if (pdpt_index < curr->pdpt)curr = curr->right;
        else if (pdpt_index > curr->pdpt)curr = curr->left;
    }

    if (pdpt_index < prev->pdpt)prev->right = pdpt;
    else if (pdpt_index > prev->pdpt)prev->left = pdpt;
}

void addPd(uint64_t cr3,uint16_t pml4_index,uint16_t pdpt_index,uint16_t pd_index) {
    if (cr3_root == 0) {
        printf("Usage of uninitialized address translater");
        abort();
    }
    struct pd_tree* pd = malloc(sizeof(struct pd_tree));
    pd->pd = pd_index;
    pd->page_size = false;
    pd->next = 0;

    struct pml4_tree* pml4 = getRoot(cr3, cr3_root)->next;
    struct pdpt_tree* pdpt = getPML4(pml4_index,pml4)->next;
    struct pdpt_tree* curr_ptr = getPDPT(pdpt_index,pdpt);
    if (curr_ptr->page_size) {
        printf("PDPT is set as page size");
        abort();
    }

    if (curr_ptr->next == 0) {
        curr_ptr->next = pd;
        return;
    }

    struct pd_tree* prev = 0;
    struct pd_tree* curr = curr_ptr->next;
    while (curr != 0) {
        prev = curr;
        if (curr->pd == pd_index) {
            printf("PD present");
            abort();
        }
        if (pd_index < curr->pd)curr = curr->right;
        else if (pd_index > curr->pd)curr = curr->left;
    }

    if (pd_index < prev->pd)prev->right = pd;
    else if (pd_index > prev->pd)prev->left = pd;
}

void addPdPage(uint64_t cr3,uint16_t pml4_index,uint16_t pdpt_index,uint16_t pd_index,void* base) {
    if (cr3_root == 0) {
        printf("Usage of uninitialized address translater");
        abort();
    }
    struct pd_tree* pd = malloc(sizeof(struct pd_tree));
    pd->pd = pd_index;
    pd->page_size = true;
    pd->next = (struct pt_tree*) base;

    struct pml4_tree* pml4 = getRoot(cr3, cr3_root)->next;
    struct pdpt_tree* pdpt = getPML4(pml4_index,pml4)->next;
    struct pdpt_tree* curr_ptr = getPDPT(pdpt_index,pdpt);
    if (curr_ptr->page_size) {
        printf("PDPT is set as page size");
        abort();
    }

    if (curr_ptr->next == 0) {
        curr_ptr->next = pd;
        return;
    }

    struct pd_tree* prev = 0;
    struct pd_tree* curr = curr_ptr->next;
    while (curr != 0) {
        prev = curr;
        if (curr->pd == pd_index) {
            printf("PD present");
            abort();
        }
        if (pd_index < curr->pd)curr = curr->right;
        else if (pd_index > curr->pd)curr = curr->left;
    }

    if (pd_index < prev->pd)prev->right = pd;
    else if (pd_index > prev->pd)prev->left = pd;
}

void addPt(uint64_t cr3,uint16_t pml4_index,uint16_t pdpt_index,uint16_t pd_index,uint16_t pt_index,void* base) {
    if (cr3_root == 0) {
        printf("Usage of uninitialized address translater");
        abort();
    }
    struct pt_tree* pt = malloc(sizeof(struct pd_tree));
    pt->pt = pt_index;
    pt->base = base;

    struct pml4_tree* pml4 = getRoot(cr3, cr3_root)->next;
    struct pdpt_tree* pdpt = getPML4(pml4_index,pml4)->next;
    struct pdpt_tree* pd_ptr = getPDPT(pdpt_index,pdpt);
    if (pd_ptr->page_size) {
        printf("PDPT is set as page size");
        abort();
    }

    struct pd_tree* pd = pd_ptr->next;
    struct pd_tree* curr_ptr = getPD(pd_index,pd);
    if (curr_ptr->page_size) {
        printf("PD is set as page size");
        abort();
    }

    if (curr_ptr->next == 0) {
        curr_ptr->next = pt;
        return;
    }

    struct pt_tree* prev = 0;
    struct pt_tree* curr = curr_ptr->next;
    while (curr != 0) {
        prev = curr;
        if (curr->pt == pt_index) {
            printf("PT present");
            abort();
        }
        if (pt_index < curr->pt)curr = curr->right;
        else if (pt_index > curr->pt)curr = curr->left;
    }

    if (pt_index < prev->pt)prev->right = pt;
    else if (pt_index > prev->pt)prev->left = pt;
}

void init_translator() {
    uint64_t size = 0;
    uint64_t cr3_phys = (uint64_t) get_page_pointer();
    addCR3(cr3_phys);
    uint64_t cr3 = cr3_phys + PAGE_VIRT_OFFSET;
    for (uint16_t i = 0;i < 512;i++) {
        uint64_t pdpt_base = ((uint64_t*)cr3)[i] & 0xFFFFFFFFFFFFF000;
        uint64_t* pdpt_table = (uint64_t*)(pdpt_base + PAGE_VIRT_OFFSET);
        if ((((uint64_t*)cr3)[i] & 0x1) == 0)continue;
        size++;
        addPML4(cr3_phys,i);
        for (uint16_t j = 0;j < 512;j++) {
            uint64_t pd_base = pdpt_table[j] & 0xFFFFFFFFFFFFF000;
            uint64_t* pd_table = (uint64_t*)(pd_base + PAGE_VIRT_OFFSET);
            if ((pdpt_table[j] & 0x1) == 0) continue;
            if ((pdpt_table[j] & 0x40) == 0x40) {
                addPdptPage(cr3_phys,i,j,(void*) (pdpt_table[j] & 0xFFFFFFFC0000000));
                size++;
                continue;
            }else {
                size++;
                addPdpt(cr3_phys,i,j);
            }

            for (uint16_t k = 0;k < 512;k++) {
                uint64_t pt_base = pd_table[k] & 0xFFFFFFFFFFFFF000;
                uint64_t* pt_table = (uint64_t*)(pt_base + PAGE_VIRT_OFFSET);
                if ((pd_table[k] & 0x1) == 0) continue;
                if ((pd_table[k] & 0x40) == 0x40) {
                    size++;
                    addPdPage(cr3_phys,i,j,k,(void*) (pd_table[k] & 0xFFFFFFFFFFE00000));
                    continue;
                }else {
                    size++;
                    addPd(cr3_phys,i,j,k);
                }

                for (uint16_t l = 0;l < 512;l++) {
                    if ((pt_table[l] & 0x1) != 0) {
                        size++;
                        addPt(cr3_phys,i,j,k,l,(void*) (pt_table[l] & 0xFFFFFFFFFFFFF000));
                    }
                }
            }
        }
    }
    volatile int j = 0;
}