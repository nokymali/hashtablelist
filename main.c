#include <stdio.h>
#include "demo_data.h"
#include "hashlist.h"

static inline __attribute__((always_inline)) void DisplayLookupResult(const char *name, void *data) {
    fprintf(stdout, "-----------------------------------------\n");
    if (NULL == data) {
        fprintf(stderr, "Lookup Failure: %s.\n", name);
    } else {
        Person *exist = (Person *)data;
        fprintf(stdout, "Lookup Success: %p, {\"%s\", \"%s\", %d}\n", exist, exist->name, exist->sex, exist->age);
    }
}

int main(int argc, char *argv[]) {
    const int prime_count = 233;

    HashListTable *hlt = HashListTableCreate(prime_count, PersonHashValue, PersonCompare, PersonFree);
    if (NULL == hlt) {
        fprintf(stderr, "Fail to Create HashListTable.\n");
        exit(EXIT_FAILURE);
    }
    const int cnt = sizeof(persons) / sizeof(persons[0]);
    for (int i = 0; i < cnt; i++) {
        const Person e = persons[i];
        Person *p = PersonAlloc(e.name, e.sex, e.age);
        if (NULL != p) {
            HashListTableAdd(hlt, (void *)p);
        }
    }
    /**< Information*/
    HashListTableInfo(hlt);

    /**< Lookup */
    Person d1 = {"Roseanne","Female",44};
    void *e = HashListTableLookup(hlt, (void *)&d1);
    DisplayLookupResult(d1.name, e);

    Person d2 = {"Sandra","Female",47};
    e = HashListTableLookup(hlt, (void *)&d2);
    DisplayLookupResult(d2.name, e);

    Person d3 = {"Cooper","Male",34};
    e = HashListTableLookup(hlt, (void *)&d3);
    DisplayLookupResult(d3.name, e);

    Person d4 = {"Sterling","Female",42};
    e = HashListTableLookup(hlt, (void *)&d4);
    DisplayLookupResult(d4.name, e);

    Person d5 = {"Dennis","Male",51};
    e = HashListTableLookup(hlt, (void *)&d5);
    DisplayLookupResult(d5.name, e);

    Person d6 = {"nokymali","Male",48};
    e = HashListTableLookup(hlt, (void *)&d6);
    DisplayLookupResult(d6.name, e);

    Person d7 = {"John", "Male",27};
    e = HashListTableLookup(hlt, (void *)&d7);
    DisplayLookupResult(d7.name, e);
    fprintf(stdout, "============================================\n");

    /**< delete */
    if (0 == HashListTableRemove(hlt, (void *)&d1)) {
        fprintf(stdout, "Remove Success {\"%s\", \"%s\", %d}\n", d1.name, d1.sex, d1.age);
        fprintf(stdout, "-----------------------------------------\n");
    }

    if (0 == HashListTableRemove(hlt, (void *)&d2)) {
        fprintf(stdout, "Remove Success {\"%s\", \"%s\", %d}\n", d2.name, d2.sex, d2.age);
        fprintf(stdout, "-----------------------------------------\n");
    }

    if (0 == HashListTableRemove(hlt, (void *)&d3)){
        fprintf(stdout, "Remove Success {\"%s\", \"%s\", %d}\n", d3.name, d3.sex, d3.age);
        fprintf(stdout, "-----------------------------------------\n");
    }

    if (0 == HashListTableRemove(hlt, (void *)&d4)) {
        fprintf(stdout, "Remove Success {\"%s\", \"%s\", %d}\n", d4.name, d4.sex, d4.age);
        fprintf(stdout, "-----------------------------------------\n");
    }

    if (0 == HashListTableRemove(hlt, (void *)&d5)) {
        fprintf(stdout, "Remove Success {\"%s\", \"%s\", %d}\n", d5.name, d5.sex, d5.age);
        fprintf(stdout, "-----------------------------------------\n");
    }

    if (0 == HashListTableRemove(hlt, (void *)&d6)) {
        fprintf(stdout, "Remove Success {\"%s\", \"%s\", %d}\n", d6.name, d6.sex, d6.age);
        fprintf(stdout, "-----------------------------------------\n");
    }

    if (0 == HashListTableRemove(hlt, (void *)&d7)){
        fprintf(stdout, "Remove Success {\"%s\", \"%s\", %d}\n", d7.name, d7.sex, d7.age);
        fprintf(stdout, "-----------------------------------------\n");
    }

    HashListTableInfo(hlt);
    HashListTableDestroy(hlt);
    return EXIT_SUCCESS;
}



