//
// Created by igel on 02.09.24.
//

#include <kernel/RSDP.h>
#include <kernel.h>
#include <kernel/memory.h>

#include <string.h>

static struct RSDT* rootRSDT = 0;

void setRSDT(struct RSDT *rsdt_n) {
    rootRSDT = (struct RSDT*) ((char*)rsdt_n + PAGE_VIRT_OFFSET);
}

void setXSDP(struct RSDP_t *xsdp) {
    unsigned char checksum = 0;
    for (uint8_t i = 0;i < sizeof(struct RSDP_t);i++) {
        checksum +=  ((char*) xsdp)[i];
    }

    if(checksum != 0) {
        hcf();
    }

    setRSDT((struct RSDT*) xsdp->RsdtAddress);
}

struct PCIe * getPCIe() {
    struct RSDT *rsdt = (struct RSDT *) rootRSDT;
    int entries = (rsdt->h.Length - sizeof(rsdt->h)) / 4;

    for (int i = 0; i < entries; i++)
    {
        struct ACPISDTHeader *h = (struct ACPISDTHeader *) ((char*) rsdt->PointerToOtherSDT[i] + PAGE_VIRT_OFFSET);
        if (!memcmp(h->Signature, "MCFG", 4))
            return (struct PCIe *) h;
    }

    // No MADT found
    return 0;
}

struct FADT* find_FADT() {
    struct RSDT *rsdt = (struct RSDT *) rootRSDT;
    int entries = (rsdt->h.Length - sizeof(rsdt->h)) / 4;

    for (int i = 0; i < entries; i++)
    {
        struct ACPISDTHeader *h = (struct ACPISDTHeader *) ((char*) rsdt->PointerToOtherSDT[i] + PAGE_VIRT_OFFSET);
        if (!memcmp(h->Signature, "FACP", 4))
            return (struct FADT *) h;
    }

    // No MADT found
    return 0;
}

struct Parsed_MADT parsed_madt() {
    struct Parsed_MADT parsed;
    memset(&parsed,0,sizeof(struct Parsed_MADT));
    struct MADT* madt = getMADT();
    if(madt == 0) {
        hcf();
    }

    uint32_t parsed_bytes = sizeof(struct ACPISDTHeader);
    char* bytes = (char*) madt;
    parsed.lapic_addr = ((uint32_t*) bytes)[parsed_bytes / sizeof(uint32_t)];
    parsed_bytes += 4;
    parsed.flags = ((uint32_t*) bytes)[parsed_bytes / sizeof(uint32_t)];
    parsed_bytes += 4;
    while (parsed_bytes < madt->header.Length) {
        parsed_bytes += 2;
        switch (bytes[parsed_bytes - 2]) {
            case 0:
                parsed.APIC_P_ID = bytes[parsed_bytes];
                parsed_bytes++;
                parsed.APIC_ID = bytes[parsed_bytes];
                parsed_bytes++;
                parsed.flags = ((uint32_t*) bytes)[parsed_bytes / sizeof(uint32_t)];
                parsed_bytes += 4;
                break;
            case 1:
                parsed.IOAPIC_ID = bytes[parsed_bytes];
                parsed_bytes += 2;//1 byte is reserverd
                parsed.IOAPIC_addr = ((uint32_t*) bytes)[parsed_bytes / sizeof(uint32_t)];
                parsed_bytes += 4;
                parsed.GSI_base = ((uint32_t*) bytes)[parsed_bytes / sizeof(uint32_t)];
                parsed_bytes += 4;
                break;
            case 2:
                parsed.bus_src = bytes[parsed_bytes];
                parsed_bytes++;
                parsed.irq_source = bytes[parsed_bytes];
                parsed_bytes++;
                parsed.gsi = ((uint32_t*) bytes)[parsed_bytes / sizeof(uint32_t)];
                parsed_bytes += 4;
                parsed.IO_Flags = ((uint16_t*) bytes)[parsed_bytes / sizeof(uint16_t)];
                parsed_bytes += 2;
                break;
            case 3:
                parsed.nmi_src = bytes[parsed_bytes];
                parsed_bytes += 2;//1 byte is reserved
                parsed.nmi_flags = ((uint16_t*) bytes)[parsed_bytes / sizeof(uint16_t)];
                parsed_bytes += 2;
                parsed.nmi_gsi = ((uint32_t*) bytes)[parsed_bytes / sizeof(uint32_t)];
                parsed_bytes += 4;
                break;
            case 4:
                parsed.LAPIC_P_ID = bytes[parsed_bytes];
                parsed_bytes++;
                parsed.LAPIC_flags = ((uint16_t*) bytes)[parsed_bytes / sizeof(uint16_t)];
                parsed_bytes += 2;
                parsed.LINT = bytes[parsed_bytes];
                parsed_bytes++;
                break;
            case 5:
                parsed_bytes += 2;
                parsed.lapic_addr = ((uint64_t*) bytes)[parsed_bytes / sizeof(uint64_t)];
                parsed_bytes += 8;
                break;
            case 9:
                parsed_bytes += 2;
                parsed.x2LAPIC_ID = ((uint32_t*) bytes)[parsed_bytes / sizeof(uint32_t)];
                parsed_bytes += 4;
                parsed.x2APIC_FLAGS = ((uint32_t*) bytes)[parsed_bytes / sizeof(uint32_t)];
                parsed_bytes += 4;
                parsed.ACPI_ID = ((uint32_t*) bytes)[parsed_bytes / sizeof(uint32_t)];
                parsed_bytes += 4;
                break;
            default:
                hcf();
                break;
        }
    }

    return parsed;
}

struct MADT * getMADT() {
    struct RSDT *rsdt = (struct RSDT *) rootRSDT;
    int entries = (rsdt->h.Length - sizeof(rsdt->h)) / 4;

    for (int i = 0; i < entries; i++)
    {
        struct ACPISDTHeader *h = (struct ACPISDTHeader *) ((char*) rsdt->PointerToOtherSDT[i] + PAGE_VIRT_OFFSET);
        if (!memcmp(h->Signature, "APIC", 4))
            return (void *) h;
    }

    // No MADT found
    return 0;
}
