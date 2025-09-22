/*
 * TermiBench - Extreme Terminal Performance Benchmark
 * 
 * A comprehensive terminal emulator benchmark suite designed to stress-test
 * terminal performance across multiple dimensions including text rendering,
 * cursor operations, scrolling, color handling, and extreme load scenarios.
 * 
 * Features:
 * - ASCII and Unicode text rendering benchmarks
 * - Color and formatting performance tests
 * - Cursor movement and positioning benchmarks
 * - Scrolling performance analysis
 * - Memory and buffer management stress tests
 * - High-frequency update scenarios
 * - Real-time performance monitoring
 * 
 * Author: Hyperion IDE Terminal Team
 * Version: 1.0.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include <math.h>

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
    #define CLEAR_SCREEN "cls"
#else
    #include <termios.h>
    #include <sys/ioctl.h>
    #define CLEAR_SCREEN "clear"
#endif

// ANSI Escape Codes
#define ESC "\033["
#define RESET ESC "0m"
#define BOLD ESC "1m"
#define DIM ESC "2m"
#define ITALIC ESC "3m"
#define UNDERLINE ESC "4m"
#define BLINK ESC "5m"
#define REVERSE ESC "7m"
#define STRIKETHROUGH ESC "9m"

// Colors
#define BLACK ESC "30m"
#define RED ESC "31m"
#define GREEN ESC "32m"
#define YELLOW ESC "33m"
#define BLUE ESC "34m"
#define MAGENTA ESC "35m"
#define CYAN ESC "36m"
#define WHITE ESC "37m"

// Background Colors
#define BG_BLACK ESC "40m"
#define BG_RED ESC "41m"
#define BG_GREEN ESC "42m"
#define BG_YELLOW ESC "43m"
#define BG_BLUE ESC "44m"
#define BG_MAGENTA ESC "45m"
#define BG_CYAN ESC "46m"
#define BG_WHITE ESC "47m"

// Cursor Operations
#define CURSOR_UP(n) ESC #n "A"
#define CURSOR_DOWN(n) ESC #n "B"
#define CURSOR_RIGHT(n) ESC #n "C"
#define CURSOR_LEFT(n) ESC #n "D"
#define CURSOR_POS(row, col) ESC #row ";" #col "H"
#define CURSOR_SAVE ESC "s"
#define CURSOR_RESTORE ESC "u"
#define CURSOR_HIDE ESC "?25l"
#define CURSOR_SHOW ESC "?25h"

// Screen Operations
#define CLEAR_SCREEN_CODE ESC "2J"
#define CLEAR_LINE ESC "2K"
#define SCROLL_UP ESC "S"
#define SCROLL_DOWN ESC "T"

// Benchmark Configuration
#define MAX_ITERATIONS 100000
#define MAX_BUFFER_SIZE 1048576  // 1MB
#define STRESS_TEST_DURATION 10  // seconds
#define PERFORMANCE_SAMPLES 1000

// Benchmark Types
typedef enum {
    BENCH_ASCII_TEXT,
    BENCH_UNICODE_TEXT,
    BENCH_COLOR_TEXT,
    BENCH_FORMATTED_TEXT,
    BENCH_CURSOR_MOVEMENT,
    BENCH_CURSOR_POSITIONING,
    BENCH_VERTICAL_SCROLL,
    BENCH_HORIZONTAL_SCROLL,
    BENCH_BUFFER_MANAGEMENT,
    BENCH_HIGH_FREQUENCY_UPDATES,
    BENCH_LARGE_DATA_VOLUME,
    BENCH_MIXED_OPERATIONS,
    BENCH_EXTREME_STRESS,
    BENCH_ALL
} benchmark_type_t;

// Performance Metrics
typedef struct {
    double min_time;
    double max_time;
    double avg_time;
    double total_time;
    long operations_count;
    double operations_per_second;
    double cpu_usage;
    size_t memory_usage;
} performance_metrics_t;

// Benchmark Result
typedef struct {
    const char* test_name;
    performance_metrics_t metrics;
    int success;
    char error_message[256];
} benchmark_result_t;

// Global Variables
static volatile int benchmark_running = 1;
static int terminal_width = 80;
static int terminal_height = 24;

// Function Prototypes
void signal_handler(int sig);
double get_time_ms(void);
void get_terminal_size(void);
void setup_terminal(void);
void restore_terminal(void);
void print_header(void);
void print_results(benchmark_result_t* results, int count);
void run_benchmark_suite(benchmark_type_t type);

// Benchmark Functions
benchmark_result_t benchmark_ascii_text(int iterations);
benchmark_result_t benchmark_unicode_text(int iterations);
benchmark_result_t benchmark_color_text(int iterations);
benchmark_result_t benchmark_formatted_text(int iterations);
benchmark_result_t benchmark_cursor_movement(int iterations);
benchmark_result_t benchmark_cursor_positioning(int iterations);
benchmark_result_t benchmark_vertical_scroll(int iterations);
benchmark_result_t benchmark_horizontal_scroll(int iterations);
benchmark_result_t benchmark_buffer_management(int iterations);
benchmark_result_t benchmark_high_frequency_updates(int duration_seconds);
benchmark_result_t benchmark_large_data_volume(size_t data_size);
benchmark_result_t benchmark_mixed_operations(int iterations);
benchmark_result_t benchmark_extreme_stress(int duration_seconds);

// Utility Functions
void generate_random_text(char* buffer, size_t size, int unicode);
void generate_random_colors(char* buffer, size_t size);
void perform_random_cursor_ops(int count);
void stress_test_output(int duration_seconds);

// Signal Handler
void signal_handler(int sig) {
    benchmark_running = 0;
    restore_terminal();
    printf("\n" RESET "Benchmark interrupted by signal %d\n", sig);
    exit(1);
}

// High-precision timer
double get_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)(tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0);
}

// Get terminal dimensions
void get_terminal_size(void) {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    terminal_width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    terminal_height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#else
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    terminal_width = w.ws_col;
    terminal_height = w.ws_row;
#endif
}

// Terminal setup
void setup_terminal(void) {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    get_terminal_size();
    
    // Enable alternative screen buffer
    printf(ESC "?1049h");
    
    // Hide cursor during benchmarks
    printf(CURSOR_HIDE);
    
    // Clear screen
    printf(CLEAR_SCREEN_CODE);
    fflush(stdout);
}

// Terminal restoration
void restore_terminal(void) {
    // Show cursor
    printf(CURSOR_SHOW);
    
    // Restore normal screen buffer
    printf(ESC "?1049l");
    
    // Reset all attributes
    printf(RESET);
    fflush(stdout);
}

// Print benchmark header
void print_header(void) {
    printf(BOLD CYAN "╔══════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                          TermiBench - Extreme Terminal Benchmark             ║\n");
    printf("║                                    Version 1.0.0                            ║\n");
    printf("╠══════════════════════════════════════════════════════════════════════════════╣\n");
    printf("║ Terminal Size: %dx%d                                                    ║\n", terminal_width, terminal_height);
    printf("║ Max Iterations: %d                                                     ║\n", MAX_ITERATIONS);
    printf("║ Stress Test Duration: %d seconds                                            ║\n", STRESS_TEST_DURATION);
    printf("╚══════════════════════════════════════════════════════════════════════════════╝" RESET "\n\n");
}

// ASCII Text Rendering Benchmark
benchmark_result_t benchmark_ascii_text(int iterations) {
    benchmark_result_t result = {"ASCII Text Rendering", {0}, 1, ""};
    
    char text_buffer[1024];
    const char* sample_text = "The quick brown fox jumps over the lazy dog. 1234567890!@#$%^&*()";
    
    double start_time = get_time_ms();
    double min_time = INFINITY, max_time = 0;
    
    for (int i = 0; i < iterations && benchmark_running; i++) {
        double iter_start = get_time_ms();
        
        // Generate random position
        int row = (rand() % (terminal_height - 2)) + 1;
        int col = (rand() % (terminal_width - 60)) + 1;
        
        // Position cursor and print text
        printf(ESC "%d;%dH%s", row, col, sample_text);
        fflush(stdout);
        
        double iter_time = get_time_ms() - iter_start;
        if (iter_time < min_time) min_time = iter_time;
        if (iter_time > max_time) max_time = iter_time;
    }
    
    double total_time = get_time_ms() - start_time;
    
    result.metrics.min_time = min_time;
    result.metrics.max_time = max_time;
    result.metrics.avg_time = total_time / iterations;
    result.metrics.total_time = total_time;
    result.metrics.operations_count = iterations;
    result.metrics.operations_per_second = (iterations * 1000.0) / total_time;
    
    return result;
}

// Unicode Text Rendering Benchmark
benchmark_result_t benchmark_unicode_text(int iterations) {
    benchmark_result_t result = {"Unicode Text Rendering", {0}, 1, ""};
    
    const char* unicode_samples[] = {
        "Hello 世界 🌍 Здравствуй мир",
        "Café naïve résumé Zürich",
        "αβγδε ñáéíóú çğşıü",
        "🚀🎉💻🔥⭐🌟💡🎯",
        "▓▒░█▄▀▐▌│┤┐└┴┬├─┼",
        "♠♣♥♦♪♫♯♭∞∑∏∆∇"
    };
    
    int sample_count = sizeof(unicode_samples) / sizeof(unicode_samples[0]);
    
    double start_time = get_time_ms();
    double min_time = INFINITY, max_time = 0;
    
    for (int i = 0; i < iterations && benchmark_running; i++) {
        double iter_start = get_time_ms();
        
        int row = (rand() % (terminal_height - 2)) + 1;
        int col = (rand() % (terminal_width - 30)) + 1;
        const char* text = unicode_samples[rand() % sample_count];
        
        printf(ESC "%d;%dH%s", row, col, text);
        fflush(stdout);
        
        double iter_time = get_time_ms() - iter_start;
        if (iter_time < min_time) min_time = iter_time;
        if (iter_time > max_time) max_time = iter_time;
    }
    
    double total_time = get_time_ms() - start_time;
    
    result.metrics.min_time = min_time;
    result.metrics.max_time = max_time;
    result.metrics.avg_time = total_time / iterations;
    result.metrics.total_time = total_time;
    result.metrics.operations_count = iterations;
    result.metrics.operations_per_second = (iterations * 1000.0) / total_time;
    
    return result;
}

// Color Text Rendering Benchmark
benchmark_result_t benchmark_color_text(int iterations) {
    benchmark_result_t result = {"Color Text Rendering", {0}, 1, ""};
    
    const char* colors[] = {RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE};
    const char* bg_colors[] = {BG_BLACK, BG_RED, BG_GREEN, BG_YELLOW, BG_BLUE, BG_MAGENTA, BG_CYAN};
    int color_count = sizeof(colors) / sizeof(colors[0]);
    int bg_count = sizeof(bg_colors) / sizeof(bg_colors[0]);
    
    double start_time = get_time_ms();
    double min_time = INFINITY, max_time = 0;
    
    for (int i = 0; i < iterations && benchmark_running; i++) {
        double iter_start = get_time_ms();
        
        int row = (rand() % (terminal_height - 2)) + 1;
        int col = (rand() % (terminal_width - 20)) + 1;
        
        const char* fg = colors[rand() % color_count];
        const char* bg = bg_colors[rand() % bg_count];
        
        printf(ESC "%d;%dH%s%sColorful Text!%s", row, col, fg, bg, RESET);
        fflush(stdout);
        
        double iter_time = get_time_ms() - iter_start;
        if (iter_time < min_time) min_time = iter_time;
        if (iter_time > max_time) max_time = iter_time;
    }
    
    double total_time = get_time_ms() - start_time;
    
    result.metrics.min_time = min_time;
    result.metrics.max_time = max_time;
    result.metrics.avg_time = total_time / iterations;
    result.metrics.total_time = total_time;
    result.metrics.operations_count = iterations;
    result.metrics.operations_per_second = (iterations * 1000.0) / total_time;
    
    return result;
}

// Formatted Text Rendering Benchmark
benchmark_result_t benchmark_formatted_text(int iterations) {
    benchmark_result_t result = {"Formatted Text Rendering", {0}, 1, ""};
    
    const char* formats[] = {BOLD, DIM, ITALIC, UNDERLINE, BLINK, REVERSE, STRIKETHROUGH};
    int format_count = sizeof(formats) / sizeof(formats[0]);
    
    double start_time = get_time_ms();
    double min_time = INFINITY, max_time = 0;
    
    for (int i = 0; i < iterations && benchmark_running; i++) {
        double iter_start = get_time_ms();
        
        int row = (rand() % (terminal_height - 2)) + 1;
        int col = (rand() % (terminal_width - 25)) + 1;
        
        const char* format = formats[rand() % format_count];
        
        printf(ESC "%d;%dH%sFormatted Text Sample%s", row, col, format, RESET);
        fflush(stdout);
        
        double iter_time = get_time_ms() - iter_start;
        if (iter_time < min_time) min_time = iter_time;
        if (iter_time > max_time) max_time = iter_time;
    }
    
    double total_time = get_time_ms() - start_time;
    
    result.metrics.min_time = min_time;
    result.metrics.max_time = max_time;
    result.metrics.avg_time = total_time / iterations;
    result.metrics.total_time = total_time;
    result.metrics.operations_count = iterations;
    result.metrics.operations_per_second = (iterations * 1000.0) / total_time;
    
    return result;
}

// Cursor Movement Benchmark
benchmark_result_t benchmark_cursor_movement(int iterations) {
    benchmark_result_t result = {"Cursor Movement", {0}, 1, ""};
    
    double start_time = get_time_ms();
    double min_time = INFINITY, max_time = 0;
    
    for (int i = 0; i < iterations && benchmark_running; i++) {
        double iter_start = get_time_ms();
        
        // Random cursor movements
        int direction = rand() % 4;
        int distance = (rand() % 10) + 1;
        
        switch (direction) {
            case 0: printf(ESC "%dA", distance); break;  // Up
            case 1: printf(ESC "%dB", distance); break;  // Down
            case 2: printf(ESC "%dC", distance); break;  // Right
            case 3: printf(ESC "%dD", distance); break;  // Left
        }
        
        fflush(stdout);
        
        double iter_time = get_time_ms() - iter_start;
        if (iter_time < min_time) min_time = iter_time;
        if (iter_time > max_time) max_time = iter_time;
    }
    
    double total_time = get_time_ms() - start_time;
    
    result.metrics.min_time = min_time;
    result.metrics.max_time = max_time;
    result.metrics.avg_time = total_time / iterations;
    result.metrics.total_time = total_time;
    result.metrics.operations_count = iterations;
    result.metrics.operations_per_second = (iterations * 1000.0) / total_time;
    
    return result;
}

// Cursor Positioning Benchmark
benchmark_result_t benchmark_cursor_positioning(int iterations) {
    benchmark_result_t result = {"Cursor Positioning", {0}, 1, ""};
    
    double start_time = get_time_ms();
    double min_time = INFINITY, max_time = 0;
    
    for (int i = 0; i < iterations && benchmark_running; i++) {
        double iter_start = get_time_ms();
        
        int row = (rand() % terminal_height) + 1;
        int col = (rand() % terminal_width) + 1;
        
        printf(ESC "%d;%dH", row, col);
        fflush(stdout);
        
        double iter_time = get_time_ms() - iter_start;
        if (iter_time < min_time) min_time = iter_time;
        if (iter_time > max_time) max_time = iter_time;
    }
    
    double total_time = get_time_ms() - start_time;
    
    result.metrics.min_time = min_time;
    result.metrics.max_time = max_time;
    result.metrics.avg_time = total_time / iterations;
    result.metrics.total_time = total_time;
    result.metrics.operations_count = iterations;
    result.metrics.operations_per_second = (iterations * 1000.0) / total_time;
    
    return result;
}

// Vertical Scrolling Benchmark
benchmark_result_t benchmark_vertical_scroll(int iterations) {
    benchmark_result_t result = {"Vertical Scrolling", {0}, 1, ""};
    
    double start_time = get_time_ms();
    double min_time = INFINITY, max_time = 0;
    
    for (int i = 0; i < iterations && benchmark_running; i++) {
        double iter_start = get_time_ms();
        
        // Fill screen with content and scroll
        for (int line = 0; line < terminal_height + 5; line++) {
            printf("Line %d: This is a test line for scrolling benchmark %d\n", line, i);
        }
        fflush(stdout);
        
        double iter_time = get_time_ms() - iter_start;
        if (iter_time < min_time) min_time = iter_time;
        if (iter_time > max_time) max_time = iter_time;
        
        // Clear for next iteration
        printf(CLEAR_SCREEN_CODE);
    }
    
    double total_time = get_time_ms() - start_time;
    
    result.metrics.min_time = min_time;
    result.metrics.max_time = max_time;
    result.metrics.avg_time = total_time / iterations;
    result.metrics.total_time = total_time;
    result.metrics.operations_count = iterations;
    result.metrics.operations_per_second = (iterations * 1000.0) / total_time;
    
    return result;
}

// High-Frequency Updates Benchmark
benchmark_result_t benchmark_high_frequency_updates(int duration_seconds) {
    benchmark_result_t result = {"High-Frequency Updates", {0}, 1, ""};
    
    double start_time = get_time_ms();
    double end_time = start_time + (duration_seconds * 1000.0);
    long operations = 0;
    double min_time = INFINITY, max_time = 0;
    
    while (get_time_ms() < end_time && benchmark_running) {
        double iter_start = get_time_ms();
        
        // Rapid updates across the screen
        for (int i = 0; i < 100; i++) {
            int row = (rand() % terminal_height) + 1;
            int col = (rand() % (terminal_width - 10)) + 1;
            printf(ESC "%d;%dH%08X", row, col, rand());
        }
        fflush(stdout);
        
        double iter_time = get_time_ms() - iter_start;
        if (iter_time < min_time) min_time = iter_time;
        if (iter_time > max_time) max_time = iter_time;
        
        operations++;
    }
    
    double total_time = get_time_ms() - start_time;
    
    result.metrics.min_time = min_time;
    result.metrics.max_time = max_time;
    result.metrics.avg_time = total_time / operations;
    result.metrics.total_time = total_time;
    result.metrics.operations_count = operations;
    result.metrics.operations_per_second = (operations * 1000.0) / total_time;
    
    return result;
}

// Large Data Volume Benchmark
benchmark_result_t benchmark_large_data_volume(size_t data_size) {
    benchmark_result_t result = {"Large Data Volume", {0}, 1, ""};
    
    char* large_buffer = malloc(data_size);
    if (!large_buffer) {
        result.success = 0;
        strcpy(result.error_message, "Failed to allocate memory for large data test");
        return result;
    }
    
    // Fill buffer with test data
    for (size_t i = 0; i < data_size - 1; i++) {
        large_buffer[i] = 'A' + (i % 26);
        if (i % 80 == 79) large_buffer[i] = '\n';
    }
    large_buffer[data_size - 1] = '\0';
    
    double start_time = get_time_ms();
    
    printf("%s", large_buffer);
    fflush(stdout);
    
    double total_time = get_time_ms() - start_time;
    
    result.metrics.min_time = total_time;
    result.metrics.max_time = total_time;
    result.metrics.avg_time = total_time;
    result.metrics.total_time = total_time;
    result.metrics.operations_count = 1;
    result.metrics.operations_per_second = 1000.0 / total_time;
    
    free(large_buffer);
    return result;
}

// Extreme Stress Test
benchmark_result_t benchmark_extreme_stress(int duration_seconds) {
    benchmark_result_t result = {"Extreme Stress Test", {0}, 1, ""};
    
    double start_time = get_time_ms();
    double end_time = start_time + (duration_seconds * 1000.0);
    long operations = 0;
    double min_time = INFINITY, max_time = 0;
    
    while (get_time_ms() < end_time && benchmark_running) {
        double iter_start = get_time_ms();
        
        // Mix of all operations
        int operation = rand() % 6;
        
        switch (operation) {
            case 0: // Text output
                printf(ESC "%d;%dH%sStress Test %ld%s", 
                       (rand() % terminal_height) + 1,
                       (rand() % (terminal_width - 20)) + 1,
                       (rand() % 2) ? RED : GREEN,
                       operations,
                       RESET);
                break;
                
            case 1: // Cursor movement
                printf(ESC "%d;%dH", 
                       (rand() % terminal_height) + 1,
                       (rand() % terminal_width) + 1);
                break;
                
            case 2: // Color changes
                printf("%s%s█%s", 
                       (rand() % 2) ? RED : BLUE,
                       (rand() % 2) ? BG_YELLOW : BG_CYAN,
                       RESET);
                break;
                
            case 3: // Clear operations
                if (rand() % 10 == 0) printf(CLEAR_SCREEN_CODE);
                else printf(CLEAR_LINE);
                break;
                
            case 4: // Unicode output
                printf("🔥⚡💻");
                break;
                
            case 5: // Formatted text
                printf("%s%sBENCH%s", BOLD, UNDERLINE, RESET);
                break;
        }
        
        if (operations % 100 == 0) fflush(stdout);
        
        double iter_time = get_time_ms() - iter_start;
        if (iter_time < min_time) min_time = iter_time;
        if (iter_time > max_time) max_time = iter_time;
        
        operations++;
    }
    
    double total_time = get_time_ms() - start_time;
    
    result.metrics.min_time = min_time;
    result.metrics.max_time = max_time;
    result.metrics.avg_time = total_time / operations;
    result.metrics.total_time = total_time;
    result.metrics.operations_count = operations;
    result.metrics.operations_per_second = (operations * 1000.0) / total_time;
    
    return result;
}

// Print benchmark results
void print_results(benchmark_result_t* results, int count) {
    printf(CLEAR_SCREEN_CODE);
    printf(BOLD CYAN "╔══════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                              BENCHMARK RESULTS                              ║\n");
    printf("╠══════════════════════════════════════════════════════════════════════════════╣" RESET "\n");
    
    for (int i = 0; i < count; i++) {
        benchmark_result_t* r = &results[i];
        
        if (r->success) {
            printf(GREEN "║ %-25s" RESET " │ " YELLOW "Ops: %8ld" RESET " │ " CYAN "Avg: %8.2fms" RESET " ║\n",
                   r->test_name, r->metrics.operations_count, r->metrics.avg_time);
            printf("║                           │ " MAGENTA "OPS: %8.0f" RESET " │ " WHITE "Tot: %8.2fms" RESET " ║\n",
                   r->metrics.operations_per_second, r->metrics.total_time);
            printf("║                           │ " RED "Min: %8.2fms" RESET " │ " BLUE "Max: %8.2fms" RESET " ║\n",
                   r->metrics.min_time, r->metrics.max_time);
        } else {
            printf(RED "║ %-25s │ ERROR: %-40s ║" RESET "\n",
                   r->test_name, r->error_message);
        }
        
        if (i < count - 1) {
            printf("╠═══════════════════════════╪═══════════════════╪═══════════════════╣\n");
        }
    }
    
    printf(BOLD CYAN "╚═══════════════════════════╧═══════════════════╧═══════════════════╝" RESET "\n\n");
}

// Run benchmark suite
void run_benchmark_suite(benchmark_type_t type) {
    benchmark_result_t results[20];
    int result_count = 0;
    
    printf(CLEAR_SCREEN_CODE);
    printf(BOLD YELLOW "Starting benchmark suite...\n" RESET);
    
    if (type == BENCH_ALL || type == BENCH_ASCII_TEXT) {
        printf("Running ASCII Text Rendering benchmark...\n");
        results[result_count++] = benchmark_ascii_text(MAX_ITERATIONS / 10);
    }
    
    if (type == BENCH_ALL || type == BENCH_UNICODE_TEXT) {
        printf("Running Unicode Text Rendering benchmark...\n");
        results[result_count++] = benchmark_unicode_text(MAX_ITERATIONS / 20);
    }
    
    if (type == BENCH_ALL || type == BENCH_COLOR_TEXT) {
        printf("Running Color Text Rendering benchmark...\n");
        results[result_count++] = benchmark_color_text(MAX_ITERATIONS / 10);
    }
    
    if (type == BENCH_ALL || type == BENCH_FORMATTED_TEXT) {
        printf("Running Formatted Text Rendering benchmark...\n");
        results[result_count++] = benchmark_formatted_text(MAX_ITERATIONS / 10);
    }
    
    if (type == BENCH_ALL || type == BENCH_CURSOR_MOVEMENT) {
        printf("Running Cursor Movement benchmark...\n");
        results[result_count++] = benchmark_cursor_movement(MAX_ITERATIONS);
    }
    
    if (type == BENCH_ALL || type == BENCH_CURSOR_POSITIONING) {
        printf("Running Cursor Positioning benchmark...\n");
        results[result_count++] = benchmark_cursor_positioning(MAX_ITERATIONS);
    }
    
    if (type == BENCH_ALL || type == BENCH_VERTICAL_SCROLL) {
        printf("Running Vertical Scrolling benchmark...\n");
        results[result_count++] = benchmark_vertical_scroll(100);
    }
    
    if (type == BENCH_ALL || type == BENCH_HIGH_FREQUENCY_UPDATES) {
        printf("Running High-Frequency Updates benchmark...\n");
        results[result_count++] = benchmark_high_frequency_updates(STRESS_TEST_DURATION / 2);
    }
    
    if (type == BENCH_ALL || type == BENCH_LARGE_DATA_VOLUME) {
        printf("Running Large Data Volume benchmark...\n");
        results[result_count++] = benchmark_large_data_volume(MAX_BUFFER_SIZE / 4);
    }
    
    if (type == BENCH_ALL || type == BENCH_EXTREME_STRESS) {
        printf("Running Extreme Stress Test...\n");
        results[result_count++] = benchmark_extreme_stress(STRESS_TEST_DURATION);
    }
    
    print_results(results, result_count);
}

// Main function
int main(int argc, char* argv[]) {
    srand((unsigned int)time(NULL));
    
    benchmark_type_t bench_type = BENCH_ALL;
    
    // Parse command line arguments
    if (argc > 1) {
        if (strcmp(argv[1], "ascii") == 0) bench_type = BENCH_ASCII_TEXT;
        else if (strcmp(argv[1], "unicode") == 0) bench_type = BENCH_UNICODE_TEXT;
        else if (strcmp(argv[1], "color") == 0) bench_type = BENCH_COLOR_TEXT;
        else if (strcmp(argv[1], "format") == 0) bench_type = BENCH_FORMATTED_TEXT;
        else if (strcmp(argv[1], "cursor") == 0) bench_type = BENCH_CURSOR_MOVEMENT;
        else if (strcmp(argv[1], "position") == 0) bench_type = BENCH_CURSOR_POSITIONING;
        else if (strcmp(argv[1], "scroll") == 0) bench_type = BENCH_VERTICAL_SCROLL;
        else if (strcmp(argv[1], "frequency") == 0) bench_type = BENCH_HIGH_FREQUENCY_UPDATES;
        else if (strcmp(argv[1], "volume") == 0) bench_type = BENCH_LARGE_DATA_VOLUME;
        else if (strcmp(argv[1], "stress") == 0) bench_type = BENCH_EXTREME_STRESS;
        else if (strcmp(argv[1], "help") == 0) {
            printf("TermiBench - Extreme Terminal Performance Benchmark\n\n");
            printf("Usage: %s [test_type]\n\n", argv[0]);
            printf("Available test types:\n");
            printf("  ascii     - ASCII text rendering\n");
            printf("  unicode   - Unicode text rendering\n");
            printf("  color     - Color text rendering\n");
            printf("  format    - Formatted text rendering\n");
            printf("  cursor    - Cursor movement\n");
            printf("  position  - Cursor positioning\n");
            printf("  scroll    - Vertical scrolling\n");
            printf("  frequency - High-frequency updates\n");
            printf("  volume    - Large data volume\n");
            printf("  stress    - Extreme stress test\n");
            printf("  all       - Run all benchmarks (default)\n");
            printf("  help      - Show this help\n\n");
            return 0;
        }
    }
    
    setup_terminal();
    print_header();
    
    printf(BOLD GREEN "Press any key to start benchmarks, or Ctrl+C to exit...\n" RESET);
    getchar();
    
    run_benchmark_suite(bench_type);
    
    printf(BOLD CYAN "Benchmark completed! Press any key to exit...\n" RESET);
    getchar();
    
    restore_terminal();
    return 0;
}