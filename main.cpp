#pragma GCC optimize("Ofast")
//#pragma comment(linker, "/STACK: 20000000")
#pragma GCC target("sse,sse2,sse3,ssse3,sse4,popcnt,abm,mmx,avx,tune=native")
#define _CRT_SECURE_NO_WARNINGS
#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstring>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <queue>
#include <random>
#include <set>
#include <vector>
#include <climits>
#include "list"
//#include <bits/stdc++.h>
#include <random>

using namespace std;

enum class GameState {
    MAXIMIZING_WON,
    MINIMIZING_WON,
    IN_PROGRESS
};

const string PASS_ACTION = "PASS_ACTION";

class miniMaxTreeSearch {

public:

  map<int,vector<int>> visited_node_time; // using hashes
  map<int,set<string>> node_action;
  vector<string> marked_actions;
  vector< vector<vector< pair<vector<string>, vector<string>> >> > automatas; // each automata is "from->to: <{action_specific_contr},{action_specific_non_contr}>"

  miniMaxTreeSearch(vector< vector<vector< pair<vector<string>, vector<string>> >> > &_automatas, vector<string> &_marked_actions) {
    automatas = _automatas; marked_actions = _marked_actions;
  }

  void solve() { // iterative deepening - todo: transposition tables
    for(int depth=0;depth<=50;depth++) {
      cerr<<"Analyzing depth "<<depth<<" ..."<<endl;
      vector< vector<vector< pair<vector<string>, vector<string>> >> > automatas_copy = automatas;
      for(auto&automata:automatas_copy) {
        automatas = {automata};
        vector<int> automata_indexes; for(auto&_:automatas) automata_indexes.push_back(0);
        int minimax_search = compute_minimax_search(automata_indexes, false, depth, INT_MIN, INT_MAX, false, 1);
        if (minimax_search==1) {cerr<<"Minimizing (aka uncontrollable) player score (solved with 1 automata): "<<minimax_search<<endl; exit(0);}
      }
      automatas = automatas_copy;
      vector<int> automata_indexes; for(auto&_:automatas) automata_indexes.push_back(0);
      int minimax_search = compute_minimax_search(automata_indexes, false, depth, INT_MIN, INT_MAX, false, 1);
      if(abs(minimax_search) == 1) { cerr<<"Minimizing (aka uncontrollable) player optimal score: "<<minimax_search<<endl; exit(0);}
    }
    cerr<<"Non conclusive result ..."<<endl;
  }

  int compute_minimax_search(vector<int>& automata_indexes, bool marked, int depth, int alpha, int beta, bool isMaximizingPlayer, int time) {

    if(depth<0) return 0;

    //string string_node = node_to_string(automata_indexes, marked);
    int node_hash = node_to_hash(automata_indexes, marked);
    set<string> _actions = get_player_actions(automata_indexes, isMaximizingPlayer);
    set<string> _all_actions = get_player_actions(automata_indexes, not isMaximizingPlayer);
    for(auto&made_action:node_action[ node_hash ])
      _actions.erase(made_action), _all_actions.erase(made_action);
    _all_actions.insert(_actions.begin(), _actions.end());
    if(_all_actions.empty()) return -1; // uncontrollable wins if no one can move

    GameState current_game_state = getGameState(node_hash, marked);
    if (current_game_state == GameState::MAXIMIZING_WON) return 1; // TODO play with "10 - depth" as score to favor faster wins or slower losses
    else if (current_game_state == GameState::MINIMIZING_WON) return -1;

    vector<string> actions(_actions.begin(),_actions.end());
    string best_action = "%%";
    random_shuffle(actions.begin(),actions.end());
    if (not isMaximizingPlayer) { // uncontrollable player - minimizing
      int best_score = INT_MAX;
      if (node_action[ node_hash ].count(PASS_ACTION) == 0)
        actions.push_back(PASS_ACTION); // uncontrollable player can also do nothing and pass
      random_shuffle(actions.begin(),actions.end());
      for(auto&action:actions) {
        // do action
        node_action[ node_hash ].insert( action );
        vector<int> next_automata_indexes = get_automata_indexes_from_action(automata_indexes, action, isMaximizingPlayer);
        bool next_marked = is_marked_action(action, marked_actions);
        if(action==PASS_ACTION) next_marked = marked; // not moving from node
        bool isMaximizingPlayerNextTurn = action==PASS_ACTION ? true : false;
        if(action != PASS_ACTION) {
          visited_node_time[ node_hash ].push_back( time );
          if(marked) marked_visits.push_back(time);
        }
        // recursive call
        int next_depth = action == PASS_ACTION ? depth : depth-1;
        int child_score = compute_minimax_search(next_automata_indexes, next_marked, next_depth, alpha, beta, isMaximizingPlayerNextTurn, time+1);
        if(child_score<best_score) best_action = action;
        best_score = min(child_score, best_score); // move to worse best move from opponent (wants to maximize) | move to best move from me (I want to minimize)
        // undo action
        node_action[ node_hash ].erase( action );
        if(action != PASS_ACTION) {
          visited_node_time[ node_hash ].pop_back();
          if(visited_node_time[node_hash].empty()) visited_node_time.erase(node_hash);
          if(marked) marked_visits.pop_back();
        }

        if (best_score<=alpha) break;
        beta = min(beta, best_score);
      }
      //cerr<<"actions uncontrolled: "; for(auto&it:actions)cerr<<it<<"#"; cerr<<endl;
      //cerr<<"best score: "<<best_score<<", action: "<<best_action<<" for state "<<node_to_string(automata_indexes, marked)<<endl;
      return best_score;
    } else { // controllable player - maximizing
      int best_score = INT_MIN;
      if(actions.empty()) actions.push_back(PASS_ACTION + "-MAX");
      for(auto&action:actions) {
        // do action
        node_action[ node_hash ].insert( action );
        vector<int> next_automata_indexes = get_automata_indexes_from_action(automata_indexes, action, isMaximizingPlayer);
        bool next_marked = is_marked_action(action, marked_actions);
        visited_node_time[ node_hash ].push_back( time );
        if(marked) marked_visits.push_back(time);
        // recursive call
        int next_depth = actions.size() == 1 ? depth : depth - 1;
        int child_score = compute_minimax_search(next_automata_indexes, next_marked, next_depth, alpha, beta, false, time+1);
        if(child_score>best_score) best_action = action;
        best_score = max(child_score, best_score);
        // undo action
        node_action[ node_hash ].erase( action );
        visited_node_time[ node_hash ].pop_back();
        if(visited_node_time[node_hash].empty()) visited_node_time.erase(node_hash);
        if(marked) marked_visits.pop_back();

        if (beta<=best_score) break;
        alpha = max(alpha, best_score);
      }
      //cerr<<"actions controlled: "; for(auto&it:actions)cerr<<it<<"#"; cerr<<endl;
      //cerr<<"best score: "<<best_score<<", action: "<<best_action<<" ,for state "<<node_to_string(automata_indexes, marked)<<endl;
      return best_score;
    }
  }

  static bool is_marked_action(const string &action, vector<string> &_marked_actions) {
    for(auto&marked_action:_marked_actions) {
      // action marked_action prefix of action?
      if(marked_action.size()>action.size())continue;
      bool is_marked = true;
      for(int i=0;i<marked_action.size();i++) {
        if(action[i]!=marked_action[i]) {is_marked=false; break;}
      }
      if(is_marked)return true;
    }
    return false;
  }

private:

    vector<int> marked_visits = {-1};

  string node_to_string(vector<int> &automata_indexes, bool marked_state) {
    string ans = "";
    for(auto&it:automata_indexes) ans += to_string(it), ans += '-';
    if(marked_state) ans += 'M';
    return ans;
  }

  int node_to_hash(vector<int> &automata_indexes, bool marked_state) {
    const int mod = 12230441;
    int base = 107;
    int actual = 1;
    int ans = 0;
    for(auto&it:automata_indexes) {
      ans += it * actual;
      ans %= mod;
      actual = (actual * base) % mod;
    }
    if(marked_state) ans += 1 * actual, ans %= mod;
    return ans;
  }

  GameState getGameState(int& node_hash, bool marked) {
    bool already_visited = visited_node_time.count(node_hash);
    if (not already_visited) return GameState::IN_PROGRESS;
    // node already visited:
    assert(visited_node_time[node_hash].size());
    if (marked) return GameState::MAXIMIZING_WON; // two passes on the same marked node
    // node already visited and not marked:
    int node_time = visited_node_time[node_hash].back();
    if ( marked_visits.back() > node_time ) { // circuit containing one marked node
      return GameState::MAXIMIZING_WON;
    } else return GameState::IN_PROGRESS; // circuit of all non-marked nodes -->
      // maybe it was because they are stuck on this circuit --> uncontrollable wins | maybe they can get out and controllable finally wins or not
      // new rule: minimizing can't do same action from node -->
  }

  set<string> get_player_actions(vector<int>& automata_state_indexes, bool isMaximizingPlayer) {
    set<string> actions;
    for(int i=0;i<automata_state_indexes.size();i++) {
      vector<pair<vector<string>, vector<string>>> &state = automatas[i][automata_state_indexes[i]];
      for (auto &to_actions: state) {
        if (isMaximizingPlayer /* uncontrollable player */ ) {
          actions.insert(to_actions.first.begin(), to_actions.first.end());
        } else { /* controllable player */
          actions.insert(to_actions.second.begin(), to_actions.second.end());
        }
      }
    }
    return actions;
  }

  vector<int> get_automata_indexes_from_action(vector<int> &automata_state_indexes, const string &action, bool isMaximizingPlayer) {
    if (action==PASS_ACTION) return automata_state_indexes;
    vector<int> next_automata_indexes = automata_state_indexes;
    for(int i=0;i<automata_state_indexes.size();i++) {
      vector< pair<vector<string>, vector<string>> > & state = automatas[i][ automata_state_indexes[i] ];
      bool action_found = false;
      for(int to=0;to<state.size();to++) {
        if(action_found)break;
        if(isMaximizingPlayer) {
          for(auto&to_action:state[to].first) {
            if(to_action==action) {action_found=true; next_automata_indexes[i] = to; break;}
          }
        }else{
          for(auto&to_action:state[to].second) {
            if(to_action==action) {action_found=true; next_automata_indexes[i] = to; break;}
          }
        }
      }
    }
    return next_automata_indexes;
  }

};

int main() {
  string format;
  cin>>format; assert(format=="#Controllable");
  int _controlled_actions; cin>>_controlled_actions;
  vector<string> controlled_actions(_controlled_actions);
  for(auto&controlled_action:controlled_actions) cin>>controlled_action;
  cin>>format; assert(format=="#Marking");
  int _marked_actions; cin>>_marked_actions;
  vector<string> marked_actions(_marked_actions);
  for(auto&marked_action:marked_actions) cin>>marked_action;
  cin>>format; assert(format=="#Machines");
  int machines; cin>>machines;
  vector< vector<vector< pair<vector<string>, vector<string>> >> > automatas(machines);
  for(auto&automata:automatas) {
    string machine_name; cin>>machine_name; // ignore this for now
    cin>>format; assert(format=="#States");
    int states; cin>>states;
    automata = vector<vector< pair<vector<string>, vector<string>> >>(states+1, vector< pair<vector<string>, vector<string> >>(states+1));
    cin>>format; assert(format=="#Edges");
    int edges;cin>>edges;
    while(edges--){
      int from, to; string action; cin>>from>>to>>action;
      if(from==-1) from=states; if(to==-1) to=states;
      if( miniMaxTreeSearch::is_marked_action(action, controlled_actions) ) // action is controlled or uncontrolled
        automata[from][to].first.push_back(action);
      else
        automata[from][to].second.push_back(action);
    }
  }
  miniMaxTreeSearch _miniMaxTreeSearch(automatas, marked_actions);
  _miniMaxTreeSearch.solve();

}

/*

  real    2m20,018s
  user    2m6,938s
  sys     0m13,019s

  (Ofast)
  real    1m45,177s
  user    1m31,348s
  sys     0m12,996s


 */