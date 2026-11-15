
#include <stdio.h>
#include "deadlock.h"

#define MAX_PROCESSES 10
#define MAX_RESOURCES 10

int allocation[MAX_PROCESSES][MAX_RESOURCES];
int max[MAX_PROCESSES][MAX_RESOURCES];
int need[MAX_PROCESSES][MAX_RESOURCES];
int available[MAX_RESOURCES];

int processes, resources;

// Calculate Need Matrix
void calculate_need(void) {
    for (int i = 0; i < processes; i++) {
        for (int j = 0; j < resources; j++) {
            need[i][j] = max[i][j] - allocation[i][j];
        }
    }
}

// Display matrices
void display_matrices(void) {
    printf("\nAllocation Matrix:\n");

    for (int i = 0; i < processes; i++) {
        for (int j = 0; j < resources; j++) {
            printf("%d ", allocation[i][j]);
        }
        printf("\n");
    }

    printf("\nMaximum Matrix:\n");

    for (int i = 0; i < processes; i++) {
        for (int j = 0; j < resources; j++) {
            printf("%d ", max[i][j]);
        }
        printf("\n");
    }

    printf("\nNeed Matrix:\n");

    for (int i = 0; i < processes; i++) {
        for (int j = 0; j < resources; j++) {
            printf("%d ", need[i][j]);
        }
        printf("\n");
    }

    printf("\nAvailable Resources:\n");

    for (int j = 0; j < resources; j++) {
        printf("%d ", available[j]);
    }

    printf("\n");
}

// Banker's Safety Algorithm
int is_safe_state(int safe_sequence[]) {

    int work[MAX_RESOURCES];
    int finish[MAX_PROCESSES] = {0};

    for (int j = 0; j < resources; j++) {
        work[j] = available[j];
    }

    int count = 0;

    while (count < processes) {

        int found = 0;

        for (int i = 0; i < processes; i++) {

            if (finish[i] == 0) {

                int possible = 1;

                for (int j = 0; j < resources; j++) {
                    if (need[i][j] > work[j]) {
                        possible = 0;
                        break;
                    }
                }

                if (possible) {

                    for (int j = 0; j < resources; j++) {
                        work[j] += allocation[i][j];
                    }

                    safe_sequence[count] = i;
                    count++;

                    finish[i] = 1;
                    found = 1;
                }
            }
        }

        if (found == 0) {
            return 0;
        }
    }

    return 1;
}

// Resource Request Algorithm
void request_resources(void) {

    int pid;
    int request[MAX_RESOURCES];

    printf("\nEnter Process Number (0 to %d): ", processes - 1);
    scanf("%d", &pid);

    if (pid < 0 || pid >= processes) {
        printf("Invalid Process Number!\n");
        return;
    }

    printf("Enter Resource Request:\n");

    for (int j = 0; j < resources; j++) {
        printf("Resource R%d: ", j);
        scanf("%d", &request[j]);
    }

    // Check request <= Need
    for (int j = 0; j < resources; j++) {
        if (request[j] > need[pid][j]) {
            printf("\nError: Request exceeds maximum declared need!\n");
            return;
        }
    }

    // Check request <= Available
    for (int j = 0; j < resources; j++) {
        if (request[j] > available[j]) {
            printf("\nResources not available. Request cannot be granted now.\n");
            return;
        }
    }

    // Temporarily allocate resources
    for (int j = 0; j < resources; j++) {
        available[j] -= request[j];
        allocation[pid][j] += request[j];
        need[pid][j] -= request[j];
    }

    int safe_sequence[MAX_PROCESSES];

    if (is_safe_state(safe_sequence)) {

        printf("\nRequest Granted Successfully!\n");
        printf("System remains in SAFE state.\n");

        printf("Safe Sequence: ");

        for (int i = 0; i < processes; i++) {
            printf("P%d", safe_sequence[i]);

            if (i < processes - 1) {
                printf(" -> ");
            }
        }

        printf("\n");

    } else {

        // Rollback allocation
        for (int j = 0; j < resources; j++) {
            available[j] += request[j];
            allocation[pid][j] -= request[j];
            need[pid][j] += request[j];
        }

        printf("\nRequest Denied!\n");
        printf("Granting this request would make the system UNSAFE.\n");
    }
}

// Main Deadlock Menu
void run_deadlock_menu(void) {

    int choice;
    int safe_sequence[MAX_PROCESSES];

    printf("\n===== Deadlock Avoidance (Banker's Algorithm) =====\n");

    printf("Enter number of processes (1-%d): ", MAX_PROCESSES);
    scanf("%d", &processes);

    printf("Enter number of resources (1-%d): ", MAX_RESOURCES);
    scanf("%d", &resources);

    if (processes < 1 || processes > MAX_PROCESSES ||
        resources < 1 || resources > MAX_RESOURCES) {

        printf("Invalid number of processes or resources!\n");
        return;
    }

    printf("\nEnter Allocation Matrix:\n");

    for (int i = 0; i < processes; i++) {
        printf("Process P%d:\n", i);

        for (int j = 0; j < resources; j++) {
            scanf("%d", &allocation[i][j]);

            if (allocation[i][j] < 0) {
                printf("Invalid negative allocation!\n");
                return;
            }
        }
    }

    printf("\nEnter Maximum Matrix:\n");

    for (int i = 0; i < processes; i++) {
        printf("Process P%d:\n", i);

        for (int j = 0; j < resources; j++) {
            scanf("%d", &max[i][j]);

            if (max[i][j] < allocation[i][j]) {
                printf("Maximum cannot be less than Allocation!\n");
                return;
            }
        }
    }

    printf("\nEnter Available Resources:\n");

    for (int j = 0; j < resources; j++) {
        scanf("%d", &available[j]);

        if (available[j] < 0) {
            printf("Invalid negative resource value!\n");
            return;
        }
    }

    calculate_need();
    display_matrices();

    if (is_safe_state(safe_sequence)) {

        printf("\nSystem is in SAFE state.\n");
        printf("Safe Sequence: ");

        for (int i = 0; i < processes; i++) {
            printf("P%d", safe_sequence[i]);

            if (i < processes - 1) {
                printf(" -> ");
            }
        }

        printf("\n");

    } else {
        printf("\nSystem is in UNSAFE state!\n");
    }

    do {

        printf("\n===== Deadlock Menu =====\n");
        printf("1. Display Matrices\n");
        printf("2. Check Safe State\n");
        printf("3. Request Resources\n");
        printf("0. Return to Main Menu\n");

        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice) {

            case 1:
                display_matrices();
                break;

            case 2:

                if (is_safe_state(safe_sequence)) {

                    printf("\nSystem is SAFE.\n");
                    printf("Safe Sequence: ");

                    for (int i = 0; i < processes; i++) {
                        printf("P%d", safe_sequence[i]);

                        if (i < processes - 1) {
                            printf(" -> ");
                        }
                    }

                    printf("\n");

                } else {
                    printf("\nSystem is UNSAFE!\n");
                }

                break;

            case 3:
                request_resources();
                break;

            case 0:
                printf("Returning to Main Menu...\n");
                break;

            default:
                printf("Invalid Choice!\n");
        }

    } while (choice != 0);
}
