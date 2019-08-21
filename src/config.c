#include <string.h>
#include <stdio.h>
#include "aes.h"
#include "utils.h"

//
// Created by baymin on 19-8-21.
//
int encryptConfig(char *filename)
{

    char* pLastSlash = strrchr(filename, '/');
    char* pszBaseName = pLastSlash ? pLastSlash + 1 : "No file";

    if (strcmp(pszBaseName, "No file") == 0) {
        error("failed to load cfg");
        return -1;
    }
    char filenamePath[256] = { 0 };//
    strncpy(filenamePath, filename, strlen(filename) - strlen(pszBaseName));

    strcat(filenamePath, "config.data");
    char key[50] = "3c69ed2d0efeb7d38108e0a96a05ffb7";
    int isOk = 0;
    isOk = DES_Encrypt(filename, key, filenamePath);
    if (isOk == OK)
    {
        printf("Encrypt cfg successed\n");
    }
    else {
        printf("failed to Encrypt cfg\n");
    }
}

int decryptConfig(char *filename)
{

    char* pLastSlash = strrchr(filename, '/');
    char* pszBaseName = pLastSlash ? pLastSlash + 1 : "No file";

    if (strcmp(pszBaseName, "No file") == 0) {
        error("failed to load cfg");
        return -1;
    }
    char filenamePath[256] = { 0 };//
    strncpy(filenamePath, filename, strlen(filename) - strlen(pszBaseName));

    strcat(filenamePath, "config-Decrypt.data");
    char key[50] = "3c69ed2d0efeb7d38108e0a96a05ffb7";
    int isOk = 0;
    isOk = DES_Decrypt(filename, key, filenamePath);
    if (isOk == OK)
    {
        printf("decrypt cfg successed\n");
    }
    else {
        printf("failed to decrypt cfg\n");
    }
}

