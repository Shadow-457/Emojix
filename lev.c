#include <stdio.h>
#include <string.h>

int levenshtein(const char *s, int ls, const char *t, int lt) {
    int a, b, c;
    if (!ls) return lt;
    if (!lt) return ls;
    int d1[lt + 1];
    int d2[lt + 1];
    int *prev = d1;
    int *curr = d2;
    for (int i = 0; i <= lt; i++) prev[i] = i;
    for (int i = 1; i <= ls; i++) {
        curr[0] = i;
        for (int j = 1; j <= lt; j++) {
            a = prev[j] + 1;
            b = curr[j - 1] + 1;
            c = prev[j - 1] + (s[i - 1] == t[j - 1] ? 0 : 1);
            int min = a < b ? a : b;
            curr[j] = min < c ? min : c;
        }
        int *temp = prev;
        prev = curr;
        curr = temp;
    }
    return prev[lt];
}

void find_best_match(const char *typed) {
    // just dummy
}
