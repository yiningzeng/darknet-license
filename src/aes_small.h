#pragma once
void ScheduleKey(unsigned char* inkey, unsigned char* outkey, int Nk, int Nr);
void AesEncrypt(unsigned char* blk, unsigned char* expansionkey, int Nr);
void Contrary_AesEncrypt(unsigned char* blk, unsigned char* expansionkey, int Nr);