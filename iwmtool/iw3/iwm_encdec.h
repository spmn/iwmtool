#ifndef _IWM_H_
#define _IWM_H_

typedef struct playerData_s
{
    unsigned char byte[2000];
    unsigned int dword[1498];
    unsigned char field_1F38[196];
} playerData_t;

typedef struct playerStats_s
{
    unsigned int checksum;
    playerData_t data;
} playerStats_t;

typedef struct iwm_s
{
    char signature[4];
    unsigned int salt;
    unsigned int hash[4];

    // hashed data
    char fs_game_path[260];
    playerStats_t stats;
} iwm_t;

static_assert(sizeof(playerData_t)  == 0x1FFC, "Invalid playerData_t size");
static_assert(sizeof(playerStats_t) == 0x2000, "Invalid playerStats_t size");
static_assert(sizeof(iwm_t)         == 0x211C, "Invalid iwm_t size");

int LiveStorage_DecryptIWMFile(iwm_t* iwm, const char* cdKey);
void LiveStorage_EncryptIWMFile(iwm_t* iwm, const char* cdKey);

#endif
