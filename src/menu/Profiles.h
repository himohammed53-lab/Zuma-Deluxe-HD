#ifndef PROFILES_H
#define PROFILES_H

/* Player accounts (offline), like the original Zuma:
   every player has a name and their own saved progress. */

#include <stdbool.h>

#define PROFILES_MAX      6
#define PROFILE_NAME_MAX 12      /* letters */
#define PROFILE_NAME_BYTES 64

typedef struct _Profiles {
    char names[PROFILES_MAX][PROFILE_NAME_BYTES];   /* UTF-8, "" = empty slot */
    int current;                                      /* -1 = nobody yet */
} Profiles;

void Profiles_Load(Profiles* p);
void Profiles_Save(const Profiles* p);

int  Profiles_Count(const Profiles* p);
/* Returns the new slot, or -1 if full / name empty / name taken */
int  Profiles_Add(Profiles* p, const char* name);
void Profiles_Delete(Profiles* p, int slot);

/* Name of the save file with this player's progress */
void Profiles_ProgressFile(int slot, char* out, int outSize);

#endif
