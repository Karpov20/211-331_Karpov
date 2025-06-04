#include <stdio.h>
#include <cstdio>
#include <iostream>
#include <tchar.h>
#include "sgx_urts.h"
#include "Enclave_u.h"
#define ENCLAVE_FILE _T("Enclave.signed.dll")

int main() {
    sgx_enclave_id_t eid;
    sgx_status_t ret = SGX_SUCCESS;
    sgx_launch_token_t token = { 0 };
    int updated = 0;

    ret = sgx_create_enclave(ENCLAVE_FILE, SGX_DEBUG_FLAG, &token, &updated, &eid, NULL);
    if (ret != SGX_SUCCESS) {
        printf("App: error %#x, failed to create enclave.\n", ret);
        return -1;
    }

    int index;
    while (true) {
        std::cout << "Enter the index of elemet: ";
        if ((std::cin >> index)) {
            char buffer[64];
            memset(buffer, '\0', 64);
            get_elem(eid, buffer, 64, index);
            printf("% s\n\n", buffer);
        }
        else {
            std::cout << "Error: Data type error\n\n";
            return 0;
        }
    }

    if (SGX_SUCCESS != sgx_destroy_enclave(eid))
        return -1;
    return 0;
}