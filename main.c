#include <stdio.h>
#include "scheduling/scheduling.h"
#include "deadlock/deadlock.h"
#include "sync/sync.h"

void print_menu(void) {
    printf("\n===== Mini OS Toolkit =====\n");
    printf("1. CPU Scheduling Simulator\n");
    printf("2. Deadlock Avoidance (Banker's Algorithm)\n");
    printf("3. Synchronization Demo\n");
    printf("4. Exit\n");
    printf("Enter your choice: ");
}

int main(void) {
    int choice;

    do {
        print_menu();

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input.\n");
            while (getchar() != '\n'); // clear bad input
            continue;
        }

        switch (choice) {
            case 1:
                run_scheduling_menu();
                break;
            case 2:
                run_deadlock_menu();
                break;
            case 3:
                run_sync_menu();
                break;
            case 4:
                printf("Exiting Mini OS Toolkit. Goodbye!\n");
                break;
            default:
                printf("Invalid choice. Try again.\n");
        }
    } while (choice != 4);

    return 0;
}