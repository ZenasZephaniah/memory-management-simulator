#include "memory_system.h"

int main() {
    initialize_system();

    std::string cmd;
    int val;

    std::cout << "\n=== MMS: Memory Management Simulator (v1.0) ===\n";

    std::cout << "Commands: malloc <size>, free <id>, dump, stats, exit\n";
    std::cout << "-----------------------------------------------\n";

    while (true) {
        std::cout << "MMS_SHELL> ";
        std::cin >> cmd;

        if (cmd == "exit") break;

        if (cmd == "malloc") {
            if (std::cin >> val) {
                sim_malloc(val);
            }
        } 
        else if (cmd == "free") {
            if (std::cin >> val) {
                sim_free(val);
            }
        }
        else if (cmd == "dump") {
            dump_memory_map();
        }

        else if (cmd == "stats") {
            print_stats();
        }

        else {
            std::cout << "Unknown command.\n";
            std::cin.clear();
            std::cin.ignore(1000, '\n');
        }
    }
    return 0;
}