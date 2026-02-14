#include <time.h>

int main() {
    time_t start = time(NULL);
    while (time(NULL) < start + 60) {
    }
}
