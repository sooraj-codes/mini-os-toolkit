#include <stdio.h>
#include <string.h>
#include "scheduling.h"

#define MAX_PROC 20
#define MAX_GANTT 5000

typedef struct {
    int pid, arrival, burst, priority;
    int remaining, completion, waiting, turnaround;
    int done;
} Process;

static Process procs[MAX_PROC];
static int n = 0;

/* Gantt chart storage: one entry per time unit (pid, or -1 for idle) */
static int gantt[MAX_GANTT];
static int gantt_len = 0;

static void clear_input(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

/* Read an integer in [min, max]; re-prompts on bad input. Exits on EOF. */
static int read_int(const char *prompt, int min, int max) {
    int v;
    for (;;) {
        printf("%s", prompt);
        int r = scanf("%d", &v);
        if (r == EOF) return 0;
        if (r == 1 && v >= min && v <= max) return v;
        printf("  Invalid input. Enter a number between %d and %d.\n", min, max);
        clear_input();
    }
}

static void reset_runtime(void) {
    for (int i = 0; i < n; i++) {
        procs[i].remaining = procs[i].burst;
        procs[i].done = 0;
        procs[i].completion = procs[i].waiting = procs[i].turnaround = 0;
    }
    gantt_len = 0;
}

static void gantt_add(int pid) {
    if (gantt_len < MAX_GANTT) gantt[gantt_len++] = pid;
}

static void input_processes(void) {
    n = read_int("Number of processes (1-20): ", 1, MAX_PROC);
    for (int i = 0; i < n; i++) {
        printf("Process P%d:\n", i + 1);
        procs[i].pid = i + 1;
        procs[i].arrival = read_int("  Arrival time (0-1000): ", 0, 1000);
        procs[i].burst = read_int("  Burst time (1-100): ", 1, 100);
        procs[i].priority = read_int("  Priority (1-10, lower = higher priority): ", 1, 10);
    }
}

static void load_sample(void) {
    int a[] = {0, 1, 2, 3, 4};
    int b[] = {8, 4, 9, 5, 2};
    int p[] = {3, 1, 4, 2, 5};
    n = 5;
    for (int i = 0; i < n; i++) {
        procs[i].pid = i + 1;
        procs[i].arrival = a[i];
        procs[i].burst = b[i];
        procs[i].priority = p[i];
    }
    printf("Sample data loaded (5 processes).\n");
}

static void show_processes(void) {
    if (n == 0) { printf("No processes entered yet.\n"); return; }
    printf("\n%-6s %-9s %-7s %-8s\n", "PID", "Arrival", "Burst", "Priority");
    for (int i = 0; i < n; i++)
        printf("P%-5d %-9d %-7d %-8d\n", procs[i].pid, procs[i].arrival,
               procs[i].burst, procs[i].priority);
}

/* Prints the Gantt chart, per-process table and averages. */
static void print_results(const char *name) {
    double tw = 0, tt = 0;

    printf("\n=== %s ===\n\nGantt Chart:\n ", name);
    for (int i = 0; i < gantt_len; i++) printf("---");
    printf("\n|");
    for (int i = 0; i < gantt_len; ) {
        int j = i;
        while (j < gantt_len && gantt[j] == gantt[i]) j++;
        int w = (j - i) * 3 - 1;
        char label[16];
        if (gantt[i] < 0) strcpy(label, "idle"); else sprintf(label, "P%d", gantt[i]);
        int len = (int)strlen(label);
        int left = (w - len) / 2; if (left < 0) left = 0;
        int right = w - len - left; if (right < 0) right = 0;
        printf("%*s%s%*s|", left, "", label, right, "");
        i = j;
    }
    printf("\n ");
    for (int i = 0; i < gantt_len; i++) printf("---");
    printf("\n0");
    for (int i = 0; i < gantt_len; ) {
        int j = i;
        while (j < gantt_len && gantt[j] == gantt[i]) j++;
        printf("%*d", (j - i) * 3, j);
        i = j;
    }
    printf("\n");

    printf("\n%-6s %-8s %-6s %-11s %-9s %-11s\n", "PID", "Arrival", "Burst",
           "Completion", "Waiting", "Turnaround");
    for (int i = 0; i < n; i++) {
        printf("P%-5d %-8d %-6d %-11d %-9d %-11d\n", procs[i].pid, procs[i].arrival,
               procs[i].burst, procs[i].completion, procs[i].waiting, procs[i].turnaround);
        tw += procs[i].waiting;
        tt += procs[i].turnaround;
    }
    printf("\nAverage Waiting Time    : %.2f\n", tw / n);
    printf("Average Turnaround Time : %.2f\n", tt / n);
}

static void finish(Process *p, int time) {
    p->done = 1;
    p->completion = time;
    p->turnaround = p->completion - p->arrival;
    p->waiting = p->turnaround - p->burst;
}

/* Non-preemptive: mode 0 = FCFS, 1 = SJF, 2 = Priority */
static void run_nonpreemptive(int mode, const char *name) {
    int time = 0, completed = 0;
    reset_runtime();
    while (completed < n) {
        int pick = -1;
        for (int i = 0; i < n; i++) {
            if (procs[i].done || procs[i].arrival > time) continue;
            if (pick < 0) { pick = i; continue; }
            int better;
            if (mode == 0) better = procs[i].arrival < procs[pick].arrival;
            else if (mode == 1) better = procs[i].burst < procs[pick].burst;
            else better = procs[i].priority < procs[pick].priority;
            if (better) pick = i;
        }
        if (pick < 0) { gantt_add(-1); time++; continue; }
        for (int k = 0; k < procs[pick].burst; k++) gantt_add(procs[pick].pid);
        time += procs[pick].burst;
        finish(&procs[pick], time);
        completed++;
    }
    print_results(name);
}

/* Preemptive: mode 1 = SRTF (shortest remaining time), 2 = Priority */
static void run_preemptive(int mode, const char *name) {
    int time = 0, completed = 0;
    reset_runtime();
    while (completed < n) {
        int pick = -1;
        for (int i = 0; i < n; i++) {
            if (procs[i].done || procs[i].arrival > time) continue;
            if (pick < 0) { pick = i; continue; }
            int better = (mode == 1) ? procs[i].remaining < procs[pick].remaining
                                     : procs[i].priority < procs[pick].priority;
            if (better) pick = i;
        }
        if (pick < 0) { gantt_add(-1); time++; continue; }
        gantt_add(procs[pick].pid);
        procs[pick].remaining--;
        time++;
        if (procs[pick].remaining == 0) {
            finish(&procs[pick], time);
            completed++;
        }
    }
    print_results(name);
}

static void enqueue_arrivals(int *queue, int *tail, int *in_queue, int time) {
    for (int i = 0; i < n; i++)
        if (!in_queue[i] && !procs[i].done && procs[i].arrival <= time) {
            queue[(*tail)++] = i;
            in_queue[i] = 1;
        }
}

static void run_rr(int quantum) {
    static int queue[MAX_PROC * 2000];
    int head = 0, tail = 0, in_queue[MAX_PROC] = {0};
    int time = 0, completed = 0;
    char name[64];

    reset_runtime();
    while (completed < n) {
        enqueue_arrivals(queue, &tail, in_queue, time);
        if (head == tail) { gantt_add(-1); time++; continue; }

        int cur = queue[head++];
        int run = procs[cur].remaining < quantum ? procs[cur].remaining : quantum;
        for (int k = 0; k < run; k++) {
            gantt_add(procs[cur].pid);
            time++;
            enqueue_arrivals(queue, &tail, in_queue, time); /* arrivals queue before the preempted process */
        }
        procs[cur].remaining -= run;
        if (procs[cur].remaining == 0) {
            finish(&procs[cur], time);
            completed++;
        } else {
            queue[tail++] = cur;
        }
    }
    sprintf(name, "Round Robin (Quantum = %d)", quantum);
    print_results(name);
}

void run_scheduling_menu(void) {
    int choice;

    printf("\n-- CPU Scheduling Simulator --\n");
    do {
        printf("\n===== Scheduling Menu =====\n");
        printf("1. Enter processes manually\n");
        printf("2. Load sample data\n");
        printf("3. Show processes\n");
        printf("4. FCFS\n");
        printf("5. SJF (Non-preemptive)\n");
        printf("6. SJF (Preemptive / SRTF)\n");
        printf("7. Round Robin\n");
        printf("8. Priority (Non-preemptive)\n");
        printf("9. Priority (Preemptive)\n");
        printf("0. Return to Main Menu\n");
        choice = read_int("Enter choice: ", 0, 9);

        if (choice >= 4 && n == 0) {
            printf("Please enter or load processes first (option 1 or 2).\n");
            continue;
        }
        switch (choice) {
            case 1: input_processes(); break;
            case 2: load_sample(); break;
            case 3: show_processes(); break;
            case 4: run_nonpreemptive(0, "FCFS (First Come First Served)"); break;
            case 5: run_nonpreemptive(1, "SJF (Non-preemptive)"); break;
            case 6: run_preemptive(1, "SJF (Preemptive / SRTF)"); break;
            case 7: run_rr(read_int("Time quantum (1-20): ", 1, 20)); break;
            case 8: run_nonpreemptive(2, "Priority Scheduling (Non-preemptive)"); break;
            case 9: run_preemptive(2, "Priority Scheduling (Preemptive)"); break;
            case 0: printf("Returning to Main Menu...\n"); break;
        }
    } while (choice != 0);
}
