#ifndef GD_BAL_BIOS_SERVICES_H
#define GD_BAL_BIOS_SERVICES_H

#define RM_CS 0x00
#define RM_DS 0x00
#define PM_CS16 0x08
#define PM_DS16 0x10
#define PM_CS32 0x18
#define PM_DS32 0x20
#define PM_CS64 0x28
#define PM_DS64 PM_DS32

#ifndef __ASSEMBLER__
struct bios_registers {
    uint16_t ds;
    uint16_t es;
    uint16_t fs;
    uint16_t gs;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t eflags;
};

struct bios_service_table {
    uint32_t magic;         //!< 'BIOS'
    uint32_t size;          //!< sizeof(*this);

    uint32_t rm_stack;      //!< Pointer to real mode stack
    uint32_t rm_scratch;    //!< Pointer to real mode scratch buffer

    uint32_t far_call_ptr;  //!< Far pointer to far call entrypoint
    uint32_t int_call_ptr;  //!< Far pointer to int call entrypoint
};

extern struct bios_service_table *bios_services;

void bios_far_call(uint32_t address, struct bios_registers *regs, const void* stack_data, unsigned stack_size);
void bios_int_call(uint8_t  num,     struct bios_registers *regs);
#endif

#endif
