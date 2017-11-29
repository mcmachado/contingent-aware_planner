/* *****************************************************************************
 * Author: Marlos C. Machado
 *
 *  Sample code for computing the number of contingent bytes in a game.
 **************************************************************************** */

#include <set>
#include <iostream>
#include <ale_interface.hpp>

#ifdef __USE_SDL
  #include <SDL.h>
#endif

using namespace std;

std::vector<int> subtractRAMs(ALERAM ram1, ALERAM ram2) {
    assert(ram1.size() == ram2.size());
    std::vector<int> result(ram1.size());
    for(unsigned i = 0; i < ram1.size(); i++) {
        result[i] = ram1.get(i) - ram2.get(i);
    }
    return result;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " rom_file" << std::endl;
        return 1;
    }

    ALEInterface ale;

    // Get & Set the desired settings
    ale.setInt("frame_skip", 5);
    ale.setInt("random_seed", 123);
    ale.setInt("max_num_frames_per_episode", 18000);
    ale.setFloat("repeat_action_probability", 0.0); // we are doing planning

#ifdef __USE_SDL
    ale.setBool("display_screen", true);
    ale.setBool("sound", true);
#endif

    // Load the ROM file. (Also resets the system for new settings to take effect.)
    ale.loadROM(argv[1]);

    // Get the vector of legal actions
    ActionVect actions = ale.getLegalActionSet();

    int reward;
    ALERAM prev_ram, curr_ram;
    prev_ram = ale.getRAM();

    std::set<int> changing_bytes;
    std::vector<int> len_changing_bytes;
    std::set<int> contingent_bytes;
    std::vector<int> len_contingent_bytes;
    std::vector<std::vector<int>> difference_rams;

    while(!ale.game_over()) {
        // Try all actions
        for(unsigned a = 0; a < actions.size(); a++) {
            ale.saveState();
            reward = ale.act(actions[a]);
            curr_ram = ale.getRAM();
            difference_rams.push_back(subtractRAMs(prev_ram, curr_ram));
            ale.loadState();
        }

        // Test to see which bytes vary according to the action taken
        for(unsigned i = 0; i < curr_ram.size(); i++) {
            bool contingent = false;
            // Pairwise combination of different actions
            for(unsigned j = 0; j < difference_rams.size(); j++) {
                for(unsigned k = 0; k < difference_rams.size(); k++) {
                    if(j < k && difference_rams[j][i] - difference_rams[k][i] != 0) {
                        contingent = true;
                    }
                }
            }
            if(contingent == true){
                contingent_bytes.insert(i);
            }
        }

        // Test to see which bytes vary, regardless of contingency
        for(unsigned i = 0; i < curr_ram.size(); i++) {
            for(unsigned j = 0; j < difference_rams.size(); j++) {
                if(difference_rams[j][i] != 0) {
                    changing_bytes.insert(i);
                }
            }
        }
        // Random walk
        Action a = actions[rand() % actions.size()];
        reward = ale.act(a);
        prev_ram = curr_ram;

        // Keeping track of the evolving length
        len_contingent_bytes.push_back(contingent_bytes.size());
        len_changing_bytes.push_back(changing_bytes.size());

        difference_rams.clear();
    }

    std::cout << "contingent_bytes = [";
    for(unsigned i = 0; i < len_contingent_bytes.size(); i++) {
        std::cout << len_contingent_bytes[i] << ", ";
    }
    std::cout << "]\nchanging_bytes = [";
    for(unsigned i = 0; i < len_changing_bytes.size(); i++) {
        std::cout << len_changing_bytes[i] << ", ";
    }
    std::cout << "]\n";

    return 0;
}
