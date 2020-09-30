#include <string.h>
#include <ctype.h>
#include <time.h>

#include "iwm_encdec.h"
#include "md4.h"


// md5? --- no looks like md4
static void IWMSaltedMD4(const void* data, unsigned int len, unsigned int salt, void* out)
{
    MD4_CTX ctx;

    MD4Init(&ctx);
    MD4Update(&ctx, (unsigned char *)&salt, sizeof(salt));
    MD4Update(&ctx, (unsigned char *)data, len);
    MD4Final(out, &ctx);
}

static void IWMDeriveEncDecKey(const char* cdKey, unsigned int salt, unsigned int* encDecKey)
{
    MD4_CTX ctx;
    int i;

    char paddedCDKey[34];
    char cdKeySaltedHash[16];
    char xorredCDKeyHash_36[16];
    char xorredCDKeyHash_5C[16];
    char intermediateHash[16];

    // custom code:
    // the original code takes the padded cdkey from a global var
    memset(paddedCDKey, ' ', sizeof(paddedCDKey));
    memcpy(paddedCDKey, cdKey, 16);
    paddedCDKey[16] = paddedCDKey[32] = paddedCDKey[33] = 0;
    //

    IWMSaltedMD4(paddedCDKey, sizeof(paddedCDKey), 0x1F93AB07, cdKeySaltedHash);
    for (i = 0; i != sizeof(cdKeySaltedHash); ++i) {
        xorredCDKeyHash_36[i] = cdKeySaltedHash[i] ^ 0x36;
        xorredCDKeyHash_5C[i] = cdKeySaltedHash[i] ^ 0x5C;
    }

    MD4Init(&ctx);
    MD4Update(&ctx, xorredCDKeyHash_36, sizeof(xorredCDKeyHash_36));
    MD4Update(&ctx, (unsigned char *)&salt, sizeof(salt));
    MD4Final(intermediateHash, &ctx);

    MD4Init(&ctx);
    MD4Update(&ctx, xorredCDKeyHash_5C, sizeof(xorredCDKeyHash_5C));
    MD4Update(&ctx, intermediateHash, sizeof(intermediateHash));
    MD4Final((unsigned char *)encDecKey, &ctx);
}

// tea?
// https://www.cnblogs.com/linzheng/archive/2011/09/14/2176767.html - chinese xd
static void IWMDecrypt(unsigned int* data, const unsigned int* key)
{
    unsigned int v2; // ecx
    unsigned int v3; // eax
    signed int v4; // eax
    char v5; // zf
    unsigned int v6; // [esp+10h] [ebp-8h]
    int v7; // [esp+14h] [ebp-4h]

    v2 = *data;
    v3 = 0xB54CDA56;
    v6 = 0xB54CDA56;
    do
    {
        v7 = (v3 >> 2) & 3;
        v4 = 0x844;
        do
        {
            data[v4] -= ((v2 ^ v6) + (data[v4 - 1] ^ key[v7 ^ v4 & 3])) ^ ((16 * data[v4 - 1] ^ (v2 >> 3))
                + ((data[v4 - 1] >> 5) ^ 4 * v2));
            v2 = data[--v4 + 1];
        } while (v4);
        *data -= ((v2 ^ v6) + (data[0x844] ^ key[v7])) ^ ((16 * data[0x844] ^ (v2 >> 3))
            + ((data[0x844] >> 5) ^ 4 * v2));
        v2 = *data;
        v5 = v6 == 0x9E3779B9;
        v3 = v6 + 0x61C88647;
        v6 += 0x61C88647;
    } while (!v5);
}

static void IWMEncrypt(unsigned int* data, const unsigned int* key)
{
    unsigned int v2; // ecx
    unsigned int v3; // eax
    signed int v4; // esi
    unsigned int v5; // eax
    unsigned int v6; // eax
    unsigned int v7; // esi
    unsigned int v8; // edi
    int v9; // ebx
    unsigned int v10; // ebx
    int v11; // [esp+10h] [ebp-Ch]
    unsigned int v12; // [esp+14h] [ebp-8h]
    int v13; // [esp+18h] [ebp-4h]

    v2 = data[0x844];
    v3 = 0;
    v4 = 6;
    do
    {
        v5 = v3 + 0x9E3779B9;
        v12 = v5;
        v11 = (v5 >> 2) & 3;
        v13 = v4 - 1;
        v6 = 0;
        do
        {
            v7 = data[v6 + 1];
            v8 = (16 * v2 ^ (data[v6 + 1] >> 3)) + ((v2 >> 5) ^ 4 * v7);
            v9 = v11 ^ v6++ & 3;
            data[v6 - 1] += ((v7 ^ v12) + (v2 ^ key[v9])) ^ v8;
            v2 = data[v6 - 1];
        } while (v6 < 0x844);
        v10 = key[v11 ^ v6 & 3];
        v3 = v12;
        v4 = v13;
        data[0x844] += ((*data ^ v12) + (v2 ^ v10)) ^ ((16 * v2 ^ (*data >> 3)) + ((v2 >> 5) ^ 4 * *data));
        v2 = data[0x844];
    } while (v13);
}

int LiveStorage_DecryptIWMFile(iwm_t* iwm, const char* cdKey)
{
    unsigned int encDecKey[4];
    unsigned int hash[4];

    if (memcmp(iwm->signature, "iwm0", 4) != 0)
        return 0;

    IWMDeriveEncDecKey(cdKey, iwm->salt, encDecKey);
    IWMDecrypt(iwm->hash, encDecKey);
    IWMSaltedMD4(iwm->fs_game_path, 0x2104, iwm->salt ^ (encDecKey[2] + 0x928D764C), hash);

    if (memcmp(iwm->hash, hash, 16) != 0)
        return 0;

    // original code compares `iwm->fs_game_path` against an user-supplied string and returns false if they don't match
    return 1;
}

void LiveStorage_EncryptIWMFile(iwm_t* iwm, const char* cdKey)
{
    unsigned int encDecKey[4];

    memcpy(iwm->signature, "iwm0", 4);
    iwm->salt = (unsigned int)time(NULL); // ORIGINAL: timeGetTime()

    IWMDeriveEncDecKey(cdKey, iwm->salt, encDecKey);
    IWMSaltedMD4(iwm->fs_game_path, 0x2104, iwm->salt ^ (encDecKey[2] + 0x928D764C), iwm->hash);
    IWMEncrypt(iwm->hash, encDecKey);
}
