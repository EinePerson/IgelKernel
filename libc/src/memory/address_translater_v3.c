//
// Created by igel on 31.03.25.
//Consider making the flags not part of the entry to avoid fragmentation and instead page walk to get flags,this also saves time when changing entries as one does not have to explicitly change the flags, the downside is that flag lookups are slower




#include <stdint.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <kernel/memory.h>

struct address_space_translator* create_translator() {
    struct address_space_translator* translator = malloc(sizeof(struct address_space_translator));
    translator->size = 0;
    translator->first = 0;
    return translator;
}

uint16_t translator_get_level_flags(struct address_space_translator* translator,void* virt,uint8_t level) {
    if (level > 4 || level == 0) {
        printf("Translator flag level ranges from 4 to 1\n");
        abort();
    }
    uint64_t virtual_address = ((uint64_t) virt) & ~0xFFF;
    uint16_t pt = virtual_address >> 12 & 0xFFFF;
    uint16_t pd = virtual_address >> 21 & 0x1FF;
    uint16_t pdpd = virtual_address >> 30 & 0x1FF;
    uint16_t pml4 = virtual_address >> 39 & 0x1FF;

    uint16_t flags = 0;

    uint64_t cr3_phys = (uint64_t) get_page_pointer();
    uint64_t cr3 = (uint64_t) translator_translate_physical(translator,(void*) cr3_phys);
    uint64_t pdpt_base = ((uint64_t*)cr3)[pml4] & 0xFFFFFFFFFFFFF000;
    uint64_t* pdpt_table = (uint64_t*)(translator_translate_physical(translator,(void*) pdpt_base));
    flags = (((uint64_t*)cr3)[pml4] & 0xFFF);

    if (level == 1)return flags;

    uint64_t pd_base = pdpt_table[pdpd] & 0xFFFFFFFFFFFFF000;
    uint64_t* pd_table = (uint64_t*)(translator_translate_physical(translator,(void*) pd_base));
    flags == (((uint64_t*)pd_table)[pdpd] & 0xFFF);
    if (level == 2)return flags;
    if ((pdpt_table[pdpd] & 0x80) != 0) {
        return -1;
    }

    uint64_t pt_base = pd_table[pd] & 0xFFFFFFFFFFFFF000;
    uint64_t* pt_table = (uint64_t*)(translator_translate_physical(translator,(void*) pt_base));
    flags == (((uint64_t*)pd_table)[pd] & 0xFFF);
    if (level == 3)return flags;
    if ((pd_table[pd] & 0x80) != 0) {
        return -1;
    }

    return pt_table[pt] & 0xFFF;
}

uint64_t translator_get_flags(struct address_space_translator* translator,void* virt) {
    //TODO make translate when not standard kernel offset ; hope it works
    uint64_t virtual_address = ((uint64_t) virt) & ~0xFFF;
    uint16_t pt = virtual_address >> 12 & 0xFFFF;
    uint16_t pd = virtual_address >> 21 & 0x1FF;
    uint16_t pdpd = virtual_address >> 30 & 0x1FF;
    uint16_t pml4 = virtual_address >> 39 & 0x1FF;

    uint64_t flags = 0;

    uint64_t cr3_phys = (uint64_t) get_page_pointer();
    uint64_t cr3 = (uint64_t) translator_translate_physical(translator,(void*) cr3_phys);
    uint64_t pdpt_base = ((uint64_t*)cr3)[pml4] & 0xFFFFFFFFFFFFF000;
    uint64_t* pdpt_table = (uint64_t*)(translator_translate_physical(translator,(void*) pdpt_base));
    flags = (((uint64_t*)cr3)[pml4] & 0xFFF) << 48;

    uint64_t pd_base = pdpt_table[pdpd] & 0xFFFFFFFFFFFFF000;
    uint64_t* pd_table = (uint64_t*)(translator_translate_physical(translator,(void*) pd_base));
    flags |= (((uint64_t*)pd_table)[pdpd] & 0xFFF) << 32;
    if ((pdpt_table[pdpd] & 0x80) != 0) {
        return flags;
    }

    uint64_t pt_base = pd_table[pd] & 0xFFFFFFFFFFFFF000;
    uint64_t* pt_table = (uint64_t*)(translator_translate_physical(translator,(void*) pt_base));
    flags |= (((uint64_t*)pd_table)[pd] & 0xFFF) << 16;
    if ((pd_table[pd] & 0x80) != 0) {
        return flags;
    }

    flags |= pt_table[pt] & 0xFFF;
    return flags;
}

void translator_add_entry(struct address_space_translator* trans,void* virt,void* phys,uint64_t length) {
    uint64_t virtual_address = ((uint64_t) virt) & ~0xFFF;
    uint64_t physical_address = ((uint64_t) phys) & ~0xFFF;

    if (trans->size == 0) {
        trans->first = malloc(sizeof(struct address_space_translator_entry));
        trans->first->virtual_address = virtual_address;
        trans->first->physical_address = physical_address;
        trans->first->length = length;
        //trans->first->all_flags = flags;
        /*trans->first->flags_pml4 = pml4_flags;
        trans->first->flags_pdpt = pdpt_flags;
        trans->first->flags_pd = pd_flags;
        trans->first->flags_pt = pt_flags;*/
        trans->size++;
        trans->cache = trans->first;
        return;
    }

    /*if (trans->size == 1 && trans->first->virtual_address + trans->first->length /*size*/ /*== virtual_address && trans->first->physical_address + trans->first->length /*size*/ /*== physical_address) {
        trans->first->length += length;
        trans->cache = trans->first;
        return;
    }*/

    struct address_space_translator_entry* entry = trans->first;
    for (int i = 0; i < trans->size - 1; i++) {
        if (entry->virtual_address + entry->length /*size*/ == virtual_address && entry->physical_address + entry->length /*size*/ == physical_address /*&& entry->all_flags == flags*/) {
            entry->length += length;
            trans->cache = entry;
            return;
        }
        if (entry->next->virtual_address > virtual_address)break;
        entry = entry->next;
    }

    if (entry->virtual_address + entry->length /*size*/ == virtual_address && entry->physical_address + entry->length /*size*/ == physical_address /*&& entry->all_flags == flags*/) {
        entry->length += length;
        trans->cache = entry;
        return;
    }

    struct address_space_translator_entry* new_entry = malloc(sizeof(struct address_space_translator_entry));
    new_entry->next = entry->next;
    entry->next = new_entry;
    new_entry->virtual_address = virtual_address;
    new_entry->physical_address = physical_address;
    new_entry->length = length;
    //new_entry->all_flags = flags;
    trans->cache = new_entry;
    trans->size++;
}

void* translator_translate_virtual(struct address_space_translator* trans,void* virt) {
    uint64_t virtual_address = ((uint64_t) virt) & ~0xFFF;
    uint64_t offset = ((uint64_t) virt) & 0xFFF;

    if (trans->cache->virtual_address == virtual_address) {
        uint64_t int_offset = virtual_address - trans->cache->virtual_address;
        return (void*) (trans->cache->physical_address + int_offset + offset);
    }

    struct address_space_translator_entry* entry = trans->first;
    for (int i = 0; i < trans->size; i++) {
        if (entry->virtual_address <= virtual_address && entry->virtual_address + entry->length >= virtual_address) {
            uint64_t int_offset = virtual_address - entry->virtual_address;
            trans->cache = entry;
            return (void*) (trans->cache->physical_address + int_offset + offset);
        }
        entry = entry->next;
    }

    return (void*) MEMORY_INVALID_RETURN;
}

void* translator_translate_physical(struct address_space_translator* trans,void* phys) {
    uint64_t physical_address = ((uint64_t) phys) & ~0xFFF;
    uint64_t offset = ((uint64_t) phys) & 0xFFF;

    if (trans->cache->physical_address == physical_address) {
        uint64_t int_offset = physical_address - trans->cache->physical_address;
        return (void*) (trans->cache->virtual_address + int_offset + offset);
    }

    struct address_space_translator_entry* entry = trans->first;
    for (int i = 0; i < trans->size; i++) {
        if (entry->physical_address <= physical_address && entry->physical_address + entry->length >= physical_address) {
            uint64_t int_offset = physical_address - entry->physical_address;
            trans->cache = entry;
            return (void*) (trans->cache->virtual_address + int_offset + offset);
        }
        entry = entry->next;
    }

    return (void*) MEMORY_INVALID_RETURN;
}

bool translator_contains_immideate_virtual_addr(struct address_space_translator* trans,uint64_t addr,uint8_t level){
    uint64_t mask = ~0xFFF;
    mask <<= 9 * (4 - level);
    addr &= mask;

    if (trans->cache->virtual_address & mask == addr)return true;

    struct address_space_translator_entry* entry = trans->first;
    for (int i = 0; i < trans->size; i++) {
        if ((entry->virtual_address & mask) <= addr && entry->virtual_address + entry->length >= addr) {
            trans->cache = entry;
            return true;
        }
        entry = entry->next;
    }

    return false;
}

bool translator_contains_virtual_addr(struct address_space_translator* trans,struct Memory_Virtual_Address* addrStr,uint8_t level){
    uint64_t addr = translateAddrToInt(addrStr);
    return translator_contains_immideate_virtual_addr(trans,addr,level);
}

bool translator_contains_virtual(struct address_space_translator* trans,void* virt) {
    uint64_t virtual_address = ((uint64_t) virt) & ~0xFFF;

    if (trans->cache->virtual_address == virtual_address)return true;

    struct address_space_translator_entry* entry = trans->first;
    for (int i = 0; i < trans->size; i++) {
        if (entry->virtual_address <= virtual_address && entry->virtual_address + entry->length >= virtual_address) {
            trans->cache = entry;
            return true;
        }
        entry = entry->next;
    }

    return false;
}

/*uint16_t translator_get_flags_physically(struct address_space_translator *trans, void *phys,uint8_t index) {
    if (index > 3) {
        printf("Address translator index out of range\n");
        abort();
    }
    uint64_t physical_address = ((uint64_t) phys) & ~0xFFF;

    if (trans->cache->physical_address == physical_address) {
        uint64_t flags = trans->cache->all_flags;
        return ((uint16_t*) &flags)[index];
    }

    struct address_space_translator_entry* entry = trans->first;
    for (int i = 0; i < trans->size - 1; i++) {
        if (entry->physical_address <= physical_address && entry->physical_address + entry->length >= physical_address) {
            trans->cache = entry;
            uint64_t flags = trans->cache->all_flags;
            return ((uint16_t*) &flags)[index];
        }
        entry = entry->next;
    }

    return 0;
}*/

/*uint64_t translator_get_all_flags_virtually(struct address_space_translator *trans, void *virt) {
    uint64_t virtual_address = ((uint64_t) virt) & ~0xFFF;

    if (trans->cache->virtual_address == virtual_address) {
        uint64_t flags = trans->cache->all_flags;
        return flags;
    }

    struct address_space_translator_entry* entry = trans->first;
    for (int i = 0; i < trans->size - 1; i++) {
        if (entry->virtual_address <= virtual_address && entry->virtual_address + entry->length >= virtual_address) {
            trans->cache = entry;
            uint64_t flags = trans->cache->all_flags;
            return flags;
        }
        entry = entry->next;
    }

    return 0;
}*/

/*uint16_t translator_get_flags_virtually(struct address_space_translator *trans, void *virt,uint8_t index) {
    if (index > 3) {
        printf("Address translator index out of range\n");
        abort();
    }
    uint64_t virtual_address = ((uint64_t) virt) & ~0xFFF;

    if (trans->cache->virtual_address == virtual_address) {
        uint64_t flags = trans->cache->all_flags;
        return ((uint16_t*) &flags)[index];
    }

    struct address_space_translator_entry* entry = trans->first;
    for (int i = 0; i < trans->size - 1; i++) {
        if (entry->virtual_address <= virtual_address && entry->virtual_address + entry->length >= virtual_address) {
            trans->cache = entry;
            uint64_t flags = trans->cache->all_flags;
            return ((uint16_t*) &flags)[index];
        }
        entry = entry->next;
    }

    return 0;
}*/

bool translator_contains_physically(struct address_space_translator* trans,void* phys) {
    uint64_t physical_address = ((uint64_t) phys) & ~0xFFF;

    if (trans->cache->physical_address == physical_address)true;

    struct address_space_translator_entry* entry = trans->first;
    for (int i = 0; i < trans->size; i++) {
        if (entry->physical_address <= physical_address && entry->physical_address + entry->length >= physical_address) {
            trans->cache = entry;
            return true;
        }
        entry = entry->next;
    }

    return false;
}

void translator_remove_entry_virtual(struct address_space_translator* trans,void* virt,uint64_t length) {
    uint64_t virtual_address = ((uint64_t) virt) & ~0xFFF;

    struct address_space_translator_entry* prev = 0;
    struct address_space_translator_entry* entry = trans->first;
    for (int i = 0; i < trans->size - 1; i++) {
        if (entry->virtual_address <= virtual_address && entry->virtual_address + entry->length >= virtual_address) {
            if (entry->length == length) {
                if (i == 0)trans->first = entry->next;
                else prev->next = entry->next;
                free(entry);
                trans->size--;
            }else if (entry->virtual_address == virtual_address) {
                entry->virtual_address += length;
                entry->physical_address += length;
                entry->length -= length;
            }else if (entry->virtual_address + entry->length == virtual_address + length) {
                entry->length -= length;
            }else {
                struct address_space_translator_entry* new_entry = malloc(sizeof(struct address_space_translator_entry));
                uint64_t first_length = virtual_address - entry->virtual_address;
                uint64_t second_length = entry->length - first_length - length;
                entry->length = first_length;
                new_entry->next = entry->next;
                entry->next = new_entry;
                new_entry->virtual_address = virtual_address + length /*size*/;
                new_entry->physical_address = entry->physical_address + entry->length + length /*size*/;
                new_entry->length = second_length;
                trans->size++;
            }
        }
        prev = entry;
        entry = entry->next;
    }
}

void translator_remove_entry_physical(struct address_space_translator* trans,void* phys,uint64_t length) {
    uint64_t physical_address = ((uint64_t) phys) & ~0xFFF;

    struct address_space_translator_entry* prev = 0;
    struct address_space_translator_entry* entry = trans->first;
    for (int i = 0; i < trans->size - 1; i++) {
        if (entry->physical_address <= physical_address && entry->physical_address + entry->length >= physical_address) {
            if (entry->length == length) {
                if (i == 0)trans->first = entry->next;
                else prev->next = entry->next;
                trans->size--;
                free(entry);
            }else if (entry->physical_address == physical_address) {
                entry->virtual_address += length;
                entry->physical_address += length;
                entry->length -= length;
            }else if (entry->physical_address + entry->length == physical_address + length) {
                entry->length -= length;
            }else {
                struct address_space_translator_entry* new_entry = malloc(sizeof(struct address_space_translator_entry));
                uint64_t first_length = physical_address - entry->physical_address;
                uint64_t second_length = entry->length - first_length - length;
                entry->length = first_length;
                new_entry->next = entry->next;
                entry->next = new_entry;
                new_entry->physical_address = physical_address + length /*size*/;
                new_entry->virtual_address = entry->virtual_address + entry->length + length /*size*/;
                new_entry->length = second_length;
                trans->size++;
            }
        }
        prev = entry;
        entry = entry->next;
    }
}

void translator_init_kernel_space(struct address_space_translator* trans) {
    uint64_t size = 0;
    uint64_t cr3_phys = (uint64_t) get_page_pointer();
    uint64_t cr3 = cr3_phys + PAGE_VIRT_OFFSET;
    for (uint16_t i = 0;i < 512;i++) {
        uint64_t pdpt_base = ((uint64_t*)cr3)[i] & 0x7FFFFFFFFFFFF000;
        uint64_t* pdpt_table = (uint64_t*)(pdpt_base + PAGE_VIRT_OFFSET);
        if ((((uint64_t*)cr3)[i] & 0x1) == 0)continue;
        size++;
        for (uint16_t j = 0;j < 512;j++) {
            uint64_t pd_base = pdpt_table[j] & 0x7FFFFFFFFFFFF000;
            uint64_t* pd_table = (uint64_t*)(pd_base + PAGE_VIRT_OFFSET);
            if ((pdpt_table[j] & 0x1) == 0) continue;
            if ((pdpt_table[j] & 0x80) != 0) {
                translator_add_entry(trans,(void*) ((pdpt_table[j] & 0x7FFFFFFC0000FFF) + PAGE_VIRT_OFFSET),(void*) (pdpt_table[j] & 0xFFFFFFFFC0000000),0x40000000);
                size++;
                continue;
            }else {
                size++;
            }

            for (uint16_t k = 0;k < 512;k++) {
                uint64_t pt_base = pd_table[k] & 0x7FFFFFFFFFFFF000;
                uint64_t* pt_table = (uint64_t*)(pt_base + PAGE_VIRT_OFFSET);
                if ((pd_table[k] & 0x1) == 0) continue;
                if ((pd_table[k] & 0x80) != 0) {
                    size++;
                    translator_add_entry(trans,(void*) ((pd_table[k] & 0x7FFFFFFFFFE00FFF) + PAGE_VIRT_OFFSET),(void*) (pd_table[k] & 0xFFFFFFFFFFE00000),0x200000);
                    continue;
                }else {
                    size++;
                }

                for (uint16_t l = 0;l < 512;l++) {
                    if ((pt_table[l] & 0x1) != 0) {
                        size++;
                        translator_add_entry(trans,(void*) ((pt_table[l] & 0x7FFFFFFFFFFFFFFF) + PAGE_VIRT_OFFSET),(void*) (pt_table[l] & 0xFFFFFFFFFFFFF000),0x1000);
                    }
                }
            }
        }
    }
}