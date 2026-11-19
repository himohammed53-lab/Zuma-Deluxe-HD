#include "Profiles.h"

#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROFILES_FILE "players.dat"

void Profiles_Load(Profiles* p) {
    memset(p, 0, sizeof(*p));
    p->current = -1;

    FILE* f = fopen(PROFILES_FILE, "r");
    if (!f)
        return;

    char line[128];
    if (fgets(line, sizeof(line), f))
        p->current = atoi(line);

    for (int i = 0; i < PROFILES_MAX; i++) {
        if (!fgets(line, sizeof(line), f))
            break;
        line[strcspn(line, "\r\n")] = 0;
        if (strcmp(line, "-") == 0)
            line[0] = 0;
        SDL_strlcpy(p->names[i], line, sizeof(p->names[i]));
    }
    fclose(f);

    if (p->current < 0 || p->current >= PROFILES_MAX || !p->names[p->current][0]) {
        p->current = -1;
        for (int i = 0; i < PROFILES_MAX; i++)
            if (p->names[i][0]) { p->current = i; break; }
    }
}

void Profiles_Save(const Profiles* p) {
    FILE* f = fopen(PROFILES_FILE, "w");
    if (!f)
        return;

    fprintf(f, "%d\n", p->current);
    for (int i = 0; i < PROFILES_MAX; i++)
        fprintf(f, "%s\n", p->names[i][0] ? p->names[i] : "-");
    fclose(f);
}

int Profiles_Count(const Profiles* p) {
    int n = 0;
    for (int i = 0; i < PROFILES_MAX; i++)
        if (p->names[i][0])
            n++;
    return n;
}

int Profiles_Add(Profiles* p, const char* name) {
    if (!name[0])
        return -1;

    for (int i = 0; i < PROFILES_MAX; i++)
        if (p->names[i][0] && strcmp(p->names[i], name) == 0)
            return -1;

    for (int i = 0; i < PROFILES_MAX; i++) {
        if (!p->names[i][0]) {
            SDL_strlcpy(p->names[i], name, sizeof(p->names[i]));
            /* a new player starts from the beginning */
            char file[32];
            Profiles_ProgressFile(i, file, sizeof(file));
            remove(file);
            return i;
        }
    }
    return -1;
}

void Profiles_Delete(Profiles* p, int slot) {
    if (slot < 0 || slot >= PROFILES_MAX)
        return;

    p->names[slot][0] = 0;

    char file[32];
    Profiles_ProgressFile(slot, file, sizeof(file));
    remove(file);

    if (p->current == slot) {
        p->current = -1;
        for (int i = 0; i < PROFILES_MAX; i++)
            if (p->names[i][0]) { p->current = i; break; }
    }
}

void Profiles_ProgressFile(int slot, char* out, int outSize) {
    SDL_snprintf(out, outSize, "player%d.bin", slot + 1);
}
