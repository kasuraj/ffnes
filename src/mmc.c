// 包含头文件
#include "nes.h"
#include "log.h"

// 内部函数实现
static void mmc_switch_pbank0(MMC *mmc, int bank)
{
    bank = (bank == -1) ? (mmc->cart->prom_count - 1) : bank; // -1 is special, means the last bank
    mmc->cbus[6].membank->data = mmc->cart->buf_prom + 0x4000 * (bank % mmc->cart->prom_count);
}

static void mmc_switch_pbank1(MMC *mmc, int bank)
{
    bank = (bank == -1) ? (mmc->cart->prom_count - 1) : bank; // -1 is special, means the last bank
    mmc->cbus[7].membank->data = mmc->cart->buf_prom + 0x4000 * (bank % mmc->cart->prom_count);
}

static void mmc_switch_cbank (MMC *mmc, int bank)
{
    bank = (bank == -1) ? (mmc->cart->crom_count - 1) : bank; // -1 is special, means the last bank
    mmc->pbus[2].membank->data = mmc->cart->buf_crom + 0x2000 * (bank % mmc->cart->crom_count);
}

// 内部类型定义
typedef struct
{
    void (*init )(MMC *mmc);
    void (*free )(MMC *mmc);
    void (*reset)(MMC *mmc);
    void (*wcb0 )(MEM *pm, int addr, BYTE byte);
    void (*wcb1 )(MEM *pm, int addr, BYTE byte);
} MAPPER;

//++ mapper002 实现 ++//
static void mapper002_reset(MMC *mmc)
{
    mmc_switch_pbank0(mmc, 0); // prom0 - first bank
    mmc_switch_pbank1(mmc,-1); // prom1 - last  bank
}

static void mapper002_wcb0(MEM *pm, int addr, BYTE byte)
{
    NES *nes = container_of(pm, NES, prom0);
    mmc_switch_pbank0(&(nes->mmc), byte);
}

static void mapper002_wcb1(MEM *pm, int addr, BYTE byte)
{
    NES *nes = container_of(pm, NES, prom1);
    mmc_switch_pbank0(&(nes->mmc), byte);
}

static void mapper002_init(MMC *mmc)
{
    mmc->pbus[2].membank->type = MEM_RAM;
    mmc->pbus[2].membank->data = mmc->chrram;

    // register bus memory callback
    mmc->cbus[6].membank->w_callback = mapper002_wcb0;
    mmc->cbus[7].membank->w_callback = mapper002_wcb1;

    // reset mapper002
    mapper002_reset(mmc);
}

static void mapper002_free(MMC *mmc)
{
    // do nothing
}

static MAPPER mapper002 =
{
    mapper002_init,
    mapper002_free,
    mapper002_reset,
    mapper002_wcb0,
    mapper002_wcb1,
};
//-- mapper002 实现 --//

//++ mapper003 实现 ++//
static void mapper003_reset(MMC *mmc)
{
    mmc_switch_cbank(mmc, 0);
}

static void mapper003_wcb0(MEM *pm, int addr, BYTE byte)
{
    NES *nes = container_of(pm, NES, prom0);
    mmc_switch_cbank(&(nes->mmc), byte);
}

static void mapper003_wcb1(MEM *pm, int addr, BYTE byte)
{
    NES *nes = container_of(pm, NES, prom1);
    mmc_switch_cbank(&(nes->mmc), byte);
}

static void mapper003_init(MMC *mmc)
{
    // register bus memory callback
    mmc->cbus[6].membank->w_callback = mapper003_wcb0;
    mmc->cbus[7].membank->w_callback = mapper003_wcb1;

    // reset mapper003
    mapper003_reset(mmc);
}

static void mapper003_free(MMC *mmc)
{
    // do nothing
}

static MAPPER mapper003 =
{
    mapper003_init,
    mapper003_free,
    mapper003_reset,
    mapper003_wcb0,
    mapper003_wcb1,
};
//-- mapper003 实现 --//

//++ mapper list ++//
static MAPPER *g_mapper_list[256] =
{
    NULL,
    NULL,
    &mapper002,
    &mapper003,
    NULL,
};
//-- mapper list --//

// 函数实现
void mmc_init(MMC *mmc, CARTRIDGE *cart, BUS cbus, BUS pbus)
{
    MAPPER *mapper;
    mmc->cart   = cart;
    mmc->cbus   = cbus;
    mmc->pbus   = pbus;
    mmc->number = cartridge_get_mappercode(cart);
    mapper = g_mapper_list[mmc->number];
    if (mapper && mapper->init) mapper->init(mmc);
    log_printf("mapper number is %03d !\n", mmc->number);
}

void mmc_free(MMC *mmc)
{
    MAPPER *mapper;
    mapper = g_mapper_list[mmc->number];
    if (mapper && mapper->init) mapper->free(mmc);
}

void mmc_reset(MMC *mmc)
{
    MAPPER *mapper;
    mapper = g_mapper_list[mmc->number];
    if (mapper && mapper->reset) mapper->reset(mmc);
}
