#include "Enclave_t.h"
#include <string.h>
#include "sgx_trts.h"

const int db_size = 7;
const char* db_arr[db_size] = { "The Beatles", "Queen", "The Rolling Stones", "Kiss", "Metallica", "AC/DC", "Nirvana" };
const char error[] = "Element does not exist!";

void get_elem(char* buffer, int len, int index) {

    if (index >= db_size || index <= 0) {
        memcpy(buffer, error, strlen(error));
    }
    else
    {
        memcpy(buffer, db_arr[index], strlen(db_arr[index]));
    }

}
