/**
 * @file    main.cpp
 * @brief	  This is the only file of the Speleo Project.
 *
 *  			  The goal is to determine whether a cave can be traversed safely.
 * 			    Different modes are proposed:
 * 			    A) The user enters the map and exit paths are shown.
 * 		  	  B) The user gives a probability that some area of the map is accessible
 * 	  		     and the program outputs the probability that the cave can be crossed.
 *   			  C) The program runs in mode B) for every probability (1% increments).
 *
 *  			  The results are surprising, there seems to be a well defined threshold
 * 	  		  in the accessibility probability from which it is safe to attempt a
 * 		    	cave traverse.
 *
 * @author	Filip Slezak
 * @version 08.05.2020
 **/

#include <iostream>
#include <iomanip>
#include <random>

#include <vector>
#include <stack>


constexpr auto ENTRY = 0;
constexpr auto A{'A'}, B{'B'}, C{'C'};


bool Bernouilli(double p) {

  static std::random_device rdev;
  static std::default_random_engine reng(rdev());

  std::bernoulli_distribution B(p);
  return B(reng);

}


namespace prompt_user {

  template <class Any>
  void BoundedValue(Any& val, Any min, Any max, std::string str = "") {
    do {
      std::cout << str;
      std::cin  >> val;
    } while(val < min or val > max);
  }
};


struct Region {

  Region(int y, int x, bool obstructed)
  : y{y}, x{x}, obstructed{obstructed}, discovered{false} {}

  int  y, x;       // Coordinates
  bool obstructed; // There's space for a human
  bool discovered; // The space has been seen

  bool IsAccessible() { return not obstructed; }
  bool IsUnknown()    { return not discovered; }

};

struct Map {

  std::vector<std::vector<Region>> map;
  int size; // map.size()

  auto& operator[](int i) { return map[i]; }

};


class Speleo {

 public: // constructors etc.
  Speleo(void);
  ~Speleo(void) = default;

 public: // public methods
  void Run(void);

 private: // private fields
  char   exec_mode;     // [prompt_user] Execution mode - see TechReq
  int    sample_size;   // [prompt_user]
  double accessibility; // [prompt_user] Probability of an area being accessible

  Map cave;                // Map of the cave to analyze
  int successful_attempts; // Successful attemps at traversing the cave

 private: // private methods
  void ReadMapFromConsole(void);
  void GenerateMap(void);

  void AttemptCaveTraverse(void); /// DFS Algorithm

  void DisplayPaths(void);
  void ResetMap(void);

};

//  ==============================================================================  //

auto main(int argc, char *argv[]) -> int {

  Speleo simulation;
  simulation.Run();

}

//  ==============================================================================  //

Speleo::Speleo() {

  prompt_user::BoundedValue(exec_mode, A, C, "Mode A, B or C ? ");
  prompt_user::BoundedValue(cave.size, 1, INT_MAX, "Cave size [>0] ? ");
  switch(exec_mode) {
    case A:
      ReadMapFromConsole();
      break;
    case B:
      prompt_user::BoundedValue(accessibility, 0.0, 1.0, "Accessibility [0;1] ? ");
      // fall through
    case C:
      // fall through
    default:
      prompt_user::BoundedValue(sample_size, 1, INT_MAX, "Sample size [>0] ? ");
  }
  std::cout << std::setprecision(4) << std::fixed;

}

void Speleo::ReadMapFromConsole() {

  for(int y{0}; y < cave.size; ++y) {
    std::vector<Region> strip;
    for(int x{0}; x < cave.size; ++x) {
      bool tmp;
      std::cin >> tmp;
      strip.push_back(Region(y, x, tmp != 0));
    }
    cave.map.push_back(strip);
  }
}

void Speleo::GenerateMap() {

  for(int y{0}; y < cave.size; ++y) {
    std::vector<Region> strip;
    for(int x{0}; x < cave.size; ++x) {
      strip.push_back(Region(y, x, Bernouilli(1 - accessibility)));
    }
    cave.map.push_back(strip);
  }
}

//  ==============================================================================  //

void Speleo::Run() {

  successful_attempts = 0;
  switch(exec_mode) {
    case A:  // elementary
      AttemptCaveTraverse();
      std::cout << (successful_attempts > 0 ? "Exit found\n" : "Exit NOT found\n");
      DisplayPaths();
      break;
    case B:  // intermediate
      for(auto sample{0}; sample < sample_size; sample += 1) {
        GenerateMap();
        AttemptCaveTraverse();
        ResetMap();
      }
      std::cout << "Success for accessibility "<< accessibility <<" is "
                << double(successful_attempts)/sample_size <<"\n";
      break;
    case C:  // advanced
      for(accessibility = 0.0, exec_mode = B; accessibility <= 1.0; accessibility += 0.01) {
        Speleo::Run();
      }
      break;
    default:
      ; // impossible situation
  }
}

void Speleo::AttemptCaveTraverse() {

  std::stack<Region> regions_to_scout;
  auto ScoutRegion = [&regions_to_scout](Region& region) {
    if(region.IsAccessible() and region.IsUnknown()) {
      region.discovered = true;
      regions_to_scout.push(region);
    }
  };
  // Find cave entries
  for(auto region : cave[ENTRY]) {
    ScoutRegion(region);
  }
  // Explore paths (starts NORTH_EAST)
  while(!regions_to_scout.empty()) {
    Region region = regions_to_scout.top();
    int y(region.y), x(region.x);
    // Stop explorations if cave exit found, in modes B and C
    if(y+1 == cave.size) {
      successful_attempts += 1;
      if(exec_mode != A) break;
    }
    regions_to_scout.pop();
    // Order the stack as to prioritise the SOUTH-WEST direction
    if(y-1 >= 0) ScoutRegion(cave[y-1][x]); // NORTH
    if(x+1 < cave.size) ScoutRegion(cave[y][x+1]); // EAST
    if(x-1 >= 0) ScoutRegion(cave[y][x-1]); // WEST
    if(y+1 < cave.size) ScoutRegion(cave[y+1][x]); // SOUTH
  }
}

//  ==============================================================================  //

void Speleo::DisplayPaths() {

  for(auto strip : cave.map) {
    for(auto region : strip) {
      std::cout << !region.discovered <<' ';
    }
    std::cout <<"\n";
  }
}

void Speleo::ResetMap() {

  cave.map.clear();

}